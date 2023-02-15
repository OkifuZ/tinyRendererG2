from utils import read_objfile, wirte_objfile


file_na = 'sphere_sd2_ir3_mqeN2.obj'

tetrahedrons = []
vertices = []

obj_dic = read_objfile(file_na)
tetrahedrons = obj_dic['tetrahedrons']
vertices = obj_dic['vertices']

print("vert num: {}, tet num: {}".format(len(vertices), len(tetrahedrons)))

triangles = set()
for tet in tetrahedrons:
    tet_sort = [id for id in tet]
    tet_sort.sort()
    for face in [ 
        (tet_sort[0], tet_sort[1], tet_sort[2]),
        (tet_sort[0], tet_sort[1], tet_sort[3]),
        (tet_sort[1], tet_sort[2], tet_sort[3]),
        (tet_sort[0], tet_sort[2], tet_sort[3])]:
        triangles.add(face)
    
triangles = list(triangles); triangles.sort()
triangles = [[tri[0], tri[1], tri[2]] for tri in triangles]
print("triangle num: {}".format(len(triangles)))

wirte_objfile(file_na[:-4]+'_triangles_only', **{
        'vertices': vertices, 
        'triangles':triangles,
        'tetrahedrons': tetrahedrons
        })