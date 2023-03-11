import os
import bpy
import sys
import bmesh
from bStream import *
from bpy_extras.io_utils import ImportHelper
import numpy as np

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
        buffer.endian = "<"

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

        for child in bpy.context.scene.objects:
            if(child.type == 'MESH'):
                bpy.context.view_layer.objects.active = child
                if(child not in meshes):
                    meshes.append(child)
                if(child.active_material not in materials):
                    materials.append(child.active_material)


        model_out = bStream(path=pth)
        model_out.endian = '<'

        model_out.writeUInt32(len(meshes))

        for mesh_obj in meshes:
            mesh = mesh_obj.to_mesh()

            uv_map = mesh.uv_layers["UVMap"].data
            colors = None
            if(mesh.vertex_colors.active is not None):
                colors = mesh.vertex_colors.active.data

            model_out.writeUInt32(len(mesh.loop_triangles)*3)
            for triangle in mesh.loop_triangles:
                for idx in range(3):
                    loop = mesh.loops[triangle.loops[idx]]

                    pos = mesh.vertices[loop.vertex_index].co
                    uv = uv_map[triangle.loops[idx]].uv
                    normal = triangle.normal
                    color = (1.0, 1.0, 1.0, 1.0)

                    if(colors is not None):
                        color = colors[triangle.loops[idx]].color

                    model_out.writeFloat(pos[0])
                    model_out.writeFloat(pos[2])
                    model_out.writeFloat(-pos[1])

                    model_out.writeFloat(normal[0])
                    model_out.writeFloat(normal[2])
                    model_out.writeFloat(-normal[1])
                    
                    model_out.writeFloat(uv[0])
                    model_out.writeFloat(uv[1])

                    model_out.writeFloat(color[0])
                    model_out.writeFloat(color[1])
                    model_out.writeFloat(color[2])
                    model_out.writeFloat(color[3])


        model_out.close()

    def GeneratePrimitives(mesh, buffer, mesh_data):
        uv_map = mesh.uv_layers["UVMap"].data
        colors = None
        if "Colors" in mesh.color_layers.keys():
            colors = mesh.color_layers["Color"].data
    
        buffer.writeUInt16(len(mesh.loop_triangles))
        for triangle in mesh.loop_triangles:
            for idx in range(3):
                loop = mesh.loops[triangle.loops[idx]]
    
    
                pos = mesh.vertices[loop.vertex_index].co
                uv = uv_map[triangle.loops[idx]].uv
                normal = triangle.normal
                color = (0.0, 0.0, 0.0, 0.0)

                if(colors is not None):
                    color = colors[triangle.loops[idx]].color

                pi = -1
                uvi = -1
                noi = -1
                coi = -1

                if(uv in mesh_data['uv']):
                    uvi = mesh_data['uv'].index(uv)
                else:
                    uvi = len(mesh_data['uv'])
                    mesh_data['uv'].append(uv)
        
                
                if(pos in mesh_data['position']):
                    pi = mesh_data['position'].index(pos)
                else:
                    pi = len(mesh_data['position'])
                    mesh_data['position'].append(pos)
    
                
                if(normal in mesh_data['normal']):
                    noi = mesh_data['normal'].index(normal)
    
                else:
                    noi = len(mesh_data['normal'])
                    mesh_data['normal'].append(normal)
    
                
                if(color in mesh_data['color']):
                    coi = mesh_data['color'].index(color)
                else:
                    coi = len(mesh_data['color'])
                    mesh_data['color'].append(color)
    
                buffer.writeUInt16(pi)  # vertex
                buffer.writeUInt16(noi) # normal
                buffer.writeUInt16(uvi) # uv
                buffer.writeUInt16(coi) # color