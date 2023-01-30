import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '.'))


import bpy
from cinnamodel import Cinnamodel
from bpy_extras.io_utils import ImportHelper, ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty, FloatProperty
from bpy.types import Operator
from bStream import *


bl_info = {
    "name" : "Cinnamodel Import Export",
    "author" : "Astral-C",
    "description" : "",
    "blender" : (2, 80, 0),
    "version" : (0, 0, 1),
    "location" : "",
    "warning" : "",
    "category" : "Generic"
}


class CinnamodelImport(bpy.types.Operator, ImportHelper):
    bl_idname = "import_model.cinnamodel"
    bl_label = "Cinnamodel (.cnmdl)"
    bl_description = "Import Cinnamodel"

    filepath = bpy.props.StringProperty(subtype="FILE_PATH")
    @classmethod
    def poll(cls, context):
        return context is not None
    
    filename_ext = ".cnmdl"

    filter_glob: StringProperty(
        default="*.cnmdl",
        options={'HIDDEN'},
        maxlen=255,  
    )

    def execute(self, context):
        Cinnamodel.ImportModel(self.filepath)
        return {'FINISHED'}
    
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

class CinnamodelExport(bpy.types.Operator, ExportHelper):
    bl_idname = "export_model.cinnamodel"
    bl_label = "Cinnamodel (.cnmdl)"
    bl_description = "Export Cinnamodel"

    filepath = bpy.props.StringProperty(subtype="FILE_PATH")
    @classmethod
    def poll(cls, context):
        return context is not None
    
    filename_ext = ".cnmdl"

    filter_glob: StringProperty(
        default="*.cnmdl",
        options={'HIDDEN'},
        maxlen=255,
    )

    def execute(self, context):
        if(os.path.exists(self.filepath)):
            os.remove(self.filepath)
        
        Cinnamodel.ExportModel(self.filepath)
        return {'FINISHED'}
    
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


def menu_func_import(self, context):
    self.layout.operator(CinnamodelImport.bl_idname)

def menu_func_export(self, context):
    self.layout.operator(CinnamodelExport.bl_idname)

def register():
    bpy.utils.register_class(CinnamodelImport)
    bpy.utils.register_class(CinnamodelExport)
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)

def unregister():
    bpy.utils.unregister_class(CinnamodelImport)
    bpy.utils.unregister_class(CinnamodelExport)
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)