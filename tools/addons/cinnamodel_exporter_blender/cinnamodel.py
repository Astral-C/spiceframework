import os
import bpy
import sys
import bmesh
from bStream import *
from bpy_extras.io_utils import ImportHelper
import numpy as np

import math

class CinnaVertex():
    def __init__(self, buffer):
        self.position = [buffer.readFloat(), buffer.readFloat(), buffer.readFloat()]
        self.position = [self.position[0], self.position[2], -self.position[1]]

        self.normal = [buffer.readFloat(), buffer.readFloat(), buffer.readFloat()]

        self.uv = [buffer.readFloat(), buffer.readFloat()]

        self.color = [buffer.readFloat(), buffer.readFloat(), buffer.readFloat(), buffer.readFloat()]

class Cinnamodel():
    def ImportModel(pth):
        buffer = bStream(path=pth)
        buffer.endian = '<'
        
        mesh_count = buffer.readUInt32()

        for mesh_idx in range(0, mesh_count):
            mesh = bpy.data.meshes.new('Mesh_{0}'.format(mesh_idx))

            verts = [CinnaVertex(buffer) for x in range(buffer.readUInt32())]

            mesh.from_pydata([vertex.position for vertex in verts], [], [[(x*3), (x*3) + 1, (x*3) + 2] for x in range(0, int(len(verts) // 3))])

            bm = bmesh.new()
            bm.from_mesh(mesh)
            uv = bm.loops.layers.uv.new("UVMap")
            color = bm.loops.layers.color.new("Colors")
            
            face_idx = 0
            for face in bm.faces:
                for vtx in range(3):
                    loop = face.loops[vtx]

                    loop[uv].uv = verts[(face_idx * 3) + vtx].uv
                    loop[color] = verts[(face_idx * 3) + vtx].color

                face_idx+=1
            

            bm.to_mesh(mesh)
            bm.free()

            mesh.update()

            mesh_obj = bpy.data.objects.new('Mesh_{0}'.format(mesh_idx), mesh)
            
            bpy.context.scene.collection.objects.link(mesh_obj)        

        buffer.close()
        pass

    def ExportModel(pth):

        meshes = []
        materials = []

        vertices = []
        textures = []
        texture_data = []

        indices = []

        max_width = 0
        max_height = 0

        for child in bpy.context.scene.objects:
            if(child.type == 'MESH'):
                bpy.context.view_layer.objects.active = child
                if(child not in meshes):
                    meshes.append(child)
                
                for texture in child.to_mesh().materials:
                    if(not(texture.node_tree.nodes.get("Principled BSDF").inputs[0].links[0].from_node.image.filepath.split("/")[-1] in textures)):
                        textures.append(texture.node_tree.nodes.get("Principled BSDF").inputs[0].links[0].from_node.image)
                        texture_data.append(texture.node_tree.nodes.get("Principled BSDF").inputs[0].links[0].from_node.image)
                        #image.size[0] image.size[1]

                        w = texture.node_tree.nodes.get("Principled BSDF").inputs[0].links[0].from_node.image.size[0];
                        h = texture.node_tree.nodes.get("Principled BSDF").inputs[0].links[0].from_node.image.size[1]

                        if(w > max_width):
                            max_width = w

                        if(h > max_height):
                            max_height = h

        print(textures)

        model_out = bStream(path=pth)
        model_out.endian = '<'
        
        model_out.writeUInt32(0)
        model_out.writeUInt32(len(meshes))
        
        for mesh_obj in meshes:
            mesh = mesh_obj.to_mesh()

            uv_map = mesh.uv_layers[0].data
            colors = None
            if(mesh.vertex_colors.active is not None):
                colors = mesh.vertex_colors.active.data

            for triangle in mesh.loop_triangles:
                for idx in range(3):
                    loop = mesh.loops[triangle.loops[idx]]

                    pos = mesh.vertices[loop.vertex_index].co
                    uv = uv_map[triangle.loops[idx]].uv
                    normal = triangle.normal
                    color = (1.0, 1.0, 1.0, 1.0)
                    texture = textures.index(mesh_obj.active_material.node_tree.nodes.get("Principled BSDF").inputs[0].links[0].from_node.image)

                    rw = mesh_obj.active_material.node_tree.nodes.get("Principled BSDF").inputs[0].links[0].from_node.image.size[0] / max_width
                    rh = mesh_obj.active_material.node_tree.nodes.get("Principled BSDF").inputs[0].links[0].from_node.image.size[1] / max_height

                    if(colors is not None):
                        color = colors[triangle.loops[idx]].color

                    vtx = {
                        "position" : [pos[0], pos[1], pos[2]],
                        "normal" : [normal[0], normal[1], normal[2]],
                        "uv" : [
                            uv[0] * rw,
                            uv[1] * rh
                        ],
                        "color" : color,
                        "texture" : texture,
                        "scale_uv" : [rw, rh]
                    }

                    if(vtx in vertices):
                        indices.append(vertices.index(vtx))
                    else:
                        indices.append(len(vertices))
                        vertices.append(vtx)

        model_out.writeUInt32(len(indices))
        model_out.writeUInt32(len(vertices))

        for index in indices:
            model_out.writeUInt32(index)

        for vertex in vertices:
            model_out.writeFloat(vertex['position'][0])
            model_out.writeFloat(vertex['position'][2])
            model_out.writeFloat(vertex['position'][1])

            model_out.writeFloat(-vertex['normal'][0])
            model_out.writeFloat(-vertex['normal'][1])
            model_out.writeFloat(vertex['normal'][2])
                        
            model_out.writeFloat(vertex['uv'][0])
            model_out.writeFloat(vertex['uv'][1])

            model_out.writeFloat(vertex['color'][0])
            model_out.writeFloat(vertex['color'][1])
            model_out.writeFloat(vertex['color'][2])
            model_out.writeFloat(vertex['color'][3])

            model_out.writeFloat(vertex['scale_uv'][0])
            model_out.writeFloat(vertex['scale_uv'][1])
            
            model_out.writeUInt32(vertex['texture'])

        texture_offset = model_out.tell()

        model_out.seek(0)
        model_out.writeUInt32(texture_offset)
        model_out.seek(texture_offset)

        model_out.writeUInt32(len(textures))
        model_out.writeUInt32(max_width)
        model_out.writeUInt32(max_height)

        for texture in texture_data:
            print(f"Writing Texture {texture.name} {texture.size[0]}x{texture.size[1]}")
            model_out.writeUInt32(texture.size[0] * texture.size[1] * 4)
            model_out.writeUInt32(texture.size[0])
            model_out.writeUInt32(texture.size[1])

            for pxl in range(0, len(texture.pixels), 4):
                print(f"Progress {pxl // 4}/{len(texture.pixels) // 4}, alpha is {int(texture.pixels[pxl + 3] * 255)}", end="\r")
                model_out.writeUInt8(int(texture.pixels[pxl] * 255))
                model_out.writeUInt8(int(texture.pixels[pxl + 1] * 255))
                model_out.writeUInt8(int(texture.pixels[pxl + 2] * 255))
                a = texture.pixels[pxl + 3] * 255
                if(a <= 255):
                    model_out.writeUInt8(int(a))
                else:
                    model_out.writeUInt8(0)

            pass

        model_out.close()
