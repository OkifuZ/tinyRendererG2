
def get_tets_envelop(tetrahedrons:list):
    envelope = set()
    for tet in tetrahedrons:
        tet_sort = [id for id in tet]
        tet_sort.sort()
        for face in [ 
            (tet_sort[0], tet_sort[1], tet_sort[2]),
            (tet_sort[0], tet_sort[1], tet_sort[3]),
            (tet_sort[1], tet_sort[2], tet_sort[3]),
            (tet_sort[0], tet_sort[2], tet_sort[3])
        ]:
            # inside face will appear twice
            if face in envelope:    envelope.remove(face)
            else:                   envelope.add(face)
    return list(envelope)


def get_CCW_wind(triangles:list, vertices:list, flip=False, verbose=False):
    def get_vert(tri:tuple, id:int):
        return vertices[tri[id]-1]

    def share_edge(tri_a:tuple, tri_b:tuple) -> bool:
        num_eq = 0
        for i in range(3):
            for j in range(3):
                num_eq += 1 if get_vert(tri_a, i) == get_vert(tri_b, j) else 0
        return num_eq >= 2

    def select(tris:list, f:callable) -> list:
        selected = []
        for tri in tris:
            if f(tri): selected.append(tri)
        return selected
                
    def flip_order(t:tuple) -> tuple:
        return (t[1],t[0],t[2])

    def correct_orientation(t:tuple, tris:list) -> tuple:
        def winding_conflict(t_a:tuple, t_b:tuple) -> bool:
            for i in range(3):
                i_ = (i+1) % 3
                for j in range(3):
                    j_ = (j+1) % 3
                    if get_vert(t_a, i) == get_vert(t_b, j) and\
                        get_vert(t_a, i_) == get_vert(t_b, j_):
                        return True
            return False

        for t_adj in tris:
            if winding_conflict(t, t_adj):
                corrected_tri = flip_order(t)
                return corrected_tri
        return t

    ### main logic-------------------------------------------------------------
    remaining_inds = list(range(1, len(triangles)))
    result = []
    tris = [triangles[0] if not flip else flip_order(triangles[0])]
    while len(remaining_inds) > 0:
        isolated = True
        for ind in remaining_inds:
            cur_tri = triangles[ind]
            adj_tris = select(tris, lambda t: share_edge(cur_tri, t))
            if len(adj_tris) > 0:
                corrected_tri = correct_orientation(cur_tri, adj_tris)
                tris.append(corrected_tri)
                if verbose: print(f'cur: {cur_tri}, adj: {adj_tris}, cor: {corrected_tri}')
                remaining_inds.remove(ind)
                isolated = False
                break
            # else, tris(result) has no adjacent triangles for cur_tri
            # the only thing we could do is to change cur_tri
            # leave the current one checked later
        if isolated:
            result.append([tri for tri in tris])
            tris.clear()
            assert(len(remaining_inds) != 0)
            ind = remaining_inds[0]
            tris.append(triangles[ind])
            remaining_inds.remove(ind)
    if len(tris) != 0:
        result.append(tris)
            
    from functools import reduce
    assert(reduce(lambda x,y: x+y, 
        [len(tris_sub) for tris_sub in result]) == len(triangles))
    return result
    ### main logic-------------------------------------------------------------

from utils import read_objfile, wirte_objfile

if __name__ == '__main__':
    flip = True
    file_na = 'sphere_sd2_ir7_mqeN2.obj'
    # file_na = 'cube.obj'

    tetrahedrons = []
    vertices = []

    obj_dic = read_objfile(file_na)
    tetrahedrons = obj_dic['tetrahedrons']
    vertices = obj_dic['vertices']

    [print(v) for v in vertices]

    surface_ids = get_tets_envelop(tetrahedrons=tetrahedrons)
    

    print('ccw:')
    ccw_surface_ids_set = get_CCW_wind(surface_ids, vertices, flip)
    ccw_surface_ids = []
    for sur_id in ccw_surface_ids_set:
        if len(sur_id) > len(ccw_surface_ids):
            ccw_surface_ids = sur_id[:]
    [print(x) for x in ccw_surface_ids]

    # wirte_objfile(file_na[:-4]+'_envelope.obj', vertices, ccw_surface_ids, tetrahedrons)
    wirte_objfile(file_na[:-4]+'_envelope', **{
        'vertices': vertices, 
        'triangles':ccw_surface_ids, 
        'tetrahedrons' : tetrahedrons
        })
    # wirte_objfile('sphere_sd2_ir7_mqeN2_envelope', **{
    # 'vertices': vertices, 
    # 'triangles':surface_ids, 
    # # 'tetrahedrons' : tetrahedrons
    # })

    
