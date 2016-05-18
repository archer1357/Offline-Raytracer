bl_info = {
  "name": "MySimpleMeshExporter",
  "author": "Me",
  "version": (1,0,0),
  "blender": (2,6,2),
  "location": "File > Export",
  "description": "",
  "warning": "",
  "wiki_url": "",
  "tracker_url": "",
  "category" : "Import-Export"}

import bpy,bmesh,bpy_extras,struct,os,mathutils,functools,math,re

def object_compare(x, y):
  if x.parent == None and y.parent != None:
    return -1

  if x.parent != None and y.parent == None:
    return 1

  if x.parent == None and y.parent == None:
    return 0

  return object_compare(x.parent,y.parent)

def write_mesh_data(fh):
  objects=bpy.context.selected_objects if bpy.context.selected_objects else bpy.data.objects
  meshObjs=[ob for ob in objects if ob.type == "MESH"]
  count=0

  for ob in meshObjs:
    me=ob.data
    me.update(calc_tessface=True)
    mat=ob.matrix_world
	
	#[0,1,0, 0,0,1, 1,0,0]*m
    mat=mathutils.Matrix.Rotation(-math.pi/2.0,4,'X')*mat
    mat=mathutils.Matrix.Rotation(-math.pi/2.0,4,'Y')*mat

    # norMat=mat.normalized().to_3x3()
    norMat=mat.copy()
    norMat.invert()
    norMat.transpose()
    norMat=norMat.to_3x3()

    for faceInd, face in enumerate(me.tessfaces):
      ma=me.materials[face.material_index] if me.materials else None
      col=ma.diffuse_color if ma else [1,1,1]
      specShininess=ma.specular_hardness/511 if ma else 0.5
      specIntensity=ma.specular_intensity if ma else 0.5


      for i in [[0,1,2],[0,2,3]] if len(face.vertices)==4 else [[0,1,2]]:
        for j in i:
          v=face.vertices[j]

          #write pos (xyz)
          pos=mat*me.vertices[v].co
          b=struct.pack("3f",pos[0],pos[1],pos[2])
          fh.write(b)

          #write nor (xyz)
          nor=me.vertices[v].normal if face.use_smooth else face.normal
          nor=norMat*nor
          nor.normalize()
          b=struct.pack("3f",nor[0],nor[1],nor[2])
          fh.write(b)

        #write colour (rgb)
        b=struct.pack("3f",col[0],col[1],col[2])
        fh.write(b)

        #write specular shininess,intensity
        b=struct.pack("2f",specShininess,specIntensity)
        fh.write(b)

        #
        count=count+1

  return count

def write_scene_data(fh):

  objects=bpy.context.selected_objects if bpy.context.selected_objects else bpy.data.objects
  sorted_objects=sorted(objects,key=functools.cmp_to_key(object_compare))

  for ob in sorted_objects:
    print(ob.name)

  print("\n")

  for actionInd,action in enumerate(bpy.data.actions):
    print("action: %s %s (%g %g)"%(action.name,action.id_root,
                           action.frame_range[0],
                           action.frame_range[1]))


    for group in action.groups:
      print(" group: %s %s"%(group.name,group.select))

      for fcurve in group.channels:
        ""

    #
    for pose_marker in action.pose_markers:
      print(" pose_marker:  %s %s %r (%i %i)"%(
        pose_marker.name,pose_marker.camera.name,
        pose_marker.select,
        pose_marker.frame[0],pose_marker.frame[1]))


    #
    for fcurve in action.fcurves:
      print("\n fcurve: (%g,%g) %s %s %i"%(
        fcurve.range()[0],fcurve.range()[1],
        fcurve.extrapolation,
        fcurve.data_path,
        fcurve.array_index))

      for sampled_point in fcurve.sampled_points:
        print("  sampled_point: .")

      for modifier in fcurve.modifiers:
        print("  modifier: .")

      for keyframe in fcurve.keyframe_points:
        print("  keyframe_point:\n   (%g %g)\n   (%g %g)\n   (%g %g)\n   %s %s %s %s\n"%(
          keyframe.co[0],keyframe.co[1],
          keyframe.handle_left[0],
          keyframe.handle_left[1],
          keyframe.handle_right[0],
          keyframe.handle_right[1],
          keyframe.handle_left_type,
          keyframe.handle_right_type,
          keyframe.type,
          keyframe.interpolation))

class MyExportMeshOld(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
  bl_idname = "my_export_old.mesh";
  bl_label = "Export";
  bl_options = {'PRESET'};
  filename_ext = ".oldmesh";

  def execute(self, context):
    with open(self.filepath, 'wb') as fh:
      count=write_mesh_data(fh)

    print('Exported to "%s with %i triangles".'%(self.filepath,count))
    return {'FINISHED'};

class MyExportScene(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
  bl_idname = "my_export.scene";
  bl_label = "Export";
  bl_options = {'PRESET'};
  filename_ext = ".scene";

  def execute(self, context):
    with open(self.filepath, 'wb') as fh:
      write_scene_data(fh)

    print('Exported to "%s".'%(self.filepath))
    return {'FINISHED'};

def menu_func(self, context):
  self.layout.operator(MyExportMeshOld.bl_idname, text="My Simple Mesh (.oldmesh)");
  self.layout.operator(MyExportScene.bl_idname, text="My Simple Scene (.scene)");

def register():
  bpy.utils.register_module(__name__);
  bpy.types.INFO_MT_file_export.append(menu_func);

def unregister():
  bpy.utils.unregister_module(__name__);
  bpy.types.INFO_MT_file_export.remove(menu_func);

if __name__ == "__main__":
  register()
