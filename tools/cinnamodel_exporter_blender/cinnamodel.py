import os
import bpy
import sys
import bmesh
from bStream import *
from bpy_extras.io_utils import ImportHelper
import numpy as np


class cinnamodel_import():
    def __init__(self, pth):

        pass



class cinnamodel_export():
    def __init__(self, pth):



        pass


    def GeneratePrimitives(mesh, buffer, mesh_data):
        uv_map = mesh.uv_layers["UVMap"].data
        colors = None
        if "Colors" in mesh.color_layers.keys():
            colors = mesh.color_layers["Color"].data
    
        buffer.writeUInt16(len(mesh.loop_triangles))
        for triangle in mesh.loop_triangles:
            for idx in range(3):
                loop = mesh.loops[triangle.loops[idx]]
    
    
                vertex = mesh.vertices[loop.vertex_index].co
                uv = uv_map[triangle.loops[idx]].uv
                normal = triangle.normal
                color = (0.0, 0.0, 0.0, 0.0)

                if(colors is not None):
                    color = colors[triangle.loops[idx]].color

                vi = -1
                uvi = -1
                noi = -1
                coi = -1

                if(uv in mesh_data['uv']):
                    uvi = mesh_data['uv'].index(uv)
                else:
                    uvi = len(mesh_data['uv'])
                    mesh_data['uv'].append(uv)
        
                
                if(vertex in mesh_data['vertex']):
                    vi = mesh_data['vertex'].index(vertex)
                else:
                    vi = len(mesh_data['vertex'])
                    mesh_data['vertex'].append(vertex)
    
                
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
    
                buffer.writeUInt16(vi)  # vertex
                buffer.writeUInt16(noi) # normal
                buffer.writeUInt16(uvi) # uv
                buffer.writeUInt16(coi) # color