def wirte_objfile(obj_name:str, **kwargs):
    '''
    @ obj_name = str 
    @ ( vertices = list ) in kwargs
    @ ( triangles = list ) in kwargs
    @ ( tetrahedrons = list ) in kwargs
    '''
    vertices:list = None
    triangles:list = None
    tetrahedrons:list = None
    if kwargs.get('vertices') is not None:
        vertices = kwargs['vertices']
    if kwargs.get('triangles') is not None:
        triangles = kwargs['triangles']
    if kwargs.get('tetrahedrons') is not None:
        tetrahedrons = kwargs['tetrahedrons']

    out_na = obj_name + '.obj'
    content = ['# Author: Siyan', f'o {obj_name}']
    content.append('')

    if vertices is not None:
        content += ['v ' + f'{v[0]} {v[1]} {v[2]}' for v in vertices]
    if vertices is not None: content.append('')
    if triangles is not None:
        content += ['f ' + f'{t[0]} {t[1]} {t[2]}' for t in triangles]
    if triangles is not None: content.append('')

    if tetrahedrons is not None:
        content += ['tet ' + f'{tet[0]} {tet[1]} {tet[2]} {tet[3]}' for tet in tetrahedrons]
    if tetrahedrons is not None: content.append('')

    with open(out_na, 'w') as fp:
        fp.writelines(line + '\n' for line in content)


def read_objfile(file_na:str):
    '''
    @ return :
    {
        tetrahedrons : None/list
        vertices : None/list
        triangles : None/list
    }
    '''
    tetrahedrons = None
    vertices = None
    triangles = None

    with open(file_na, 'r') as fp:
        lines = fp.readlines()
        for line in lines:
            if line.startswith('f '):
                ids = line.strip().split(' ')[1:]
                if len(ids) == 4:
                    if tetrahedrons is None: tetrahedrons = []
                    tet_ids = [int(id) for id in ids]
                    tetrahedrons.append(tet_ids)
                if len(ids) == 3:
                    if triangles is None: triangles = []
                    tri_ids = [int(id) for id in ids]
                    triangles.append(tri_ids)
            elif line.startswith('v '):
                if vertices is None: vertices = []
                vert = line.strip().split(' ')[1:]
                vert = [float(v) for v in vert]
                vertices.append(vert)
            elif line.startswith('t '):
                if tetrahedrons is None: tetrahedrons = []
                ids = line.strip().split(' ')[1:]
                tet_ids = [int(id) for id in ids]
                tetrahedrons.append(tet_ids)
    return {
        'tetrahedrons': tetrahedrons,
        'vertices': vertices, 
        'triangles': triangles }
            
