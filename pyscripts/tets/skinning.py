from utils import read_objfile
from dataclasses import dataclass
from typing import overload

@dataclass
class SkinningInfoItem:
    vert_id : int # 1, 2, 3...
    b0 : float = float('inf')
    b1 : float = float('inf')
    b2 : float = float('inf')
    tet_id : int = -1 # 1, 2, 3...

@dataclass
class Vertices:
    d_min : float
    pos : tuple # (x, y, z)
    id : int # 1, 2, 3...

@dataclass
class BoundingBox:
    x_min : float = float('inf')
    x_max : float = -float('inf')
    y_min : float = float('inf')
    y_max : float = -float('inf')
    z_min : float = float('inf')
    z_max : float = -float('inf')

    def set_model(self, vertices:list[tuple|Vertices|list]):
        if len(vertices) == 0:
            return
        if isinstance(vertices[0], tuple) or isinstance(vertices[0], list):
            for vert_tup in vertices:
                self.x_min = min(self.x_min, vert_tup[0])
                self.x_max = max(self.x_max, vert_tup[0])
                self.y_min = min(self.y_min, vert_tup[1])
                self.y_max = max(self.y_max, vert_tup[1])
                self.z_min = min(self.z_min, vert_tup[2])
                self.z_max = max(self.z_max, vert_tup[2])
        elif isinstance(vertices[0], Vertices):
            for vert in vertices:
                self.x_min = min(self.x_min, vert.pos[0])
                self.x_max = max(self.x_max, vert.pos[0])
                self.y_min = min(self.y_min, vert.pos[1])
                self.y_max = max(self.y_max, vert.pos[1])
                self.z_min = min(self.z_min, vert.pos[2])
                self.z_max = max(self.z_max, vert.pos[2])

        


if __name__ == '__main__':
    file_na_surface = 'sphere_surface.obj'
    file_na_tets = 'sphere_sd2_ir3_mqeN2_envelope.obj'

    vertices = []
    obj_dic = read_objfile(file_na_surface)
    vertices_surface = obj_dic['vertices']

    vertices_info = [Vertices(d_min=-float('inf'), pos=(vert[0], vert[1], vert[2]), id=id+1) 
        for id, vert in enumerate(vertices_surface)]
    skinning_info = [SkinningInfoItem(vert_id=vert.id)  for vert in vertices_info]

    obj_dic = read_objfile(file_na_tets)    
    vertices_tets = obj_dic['vertices']
    tets_quad = obj_dic['tetrahedrons']


    def query() -> list[Vertices]:
        pass
    
    def barycentric() -> list[float]:
        # return b0, b1, b2
        pass
    
    # spatial hash all vertices of surface mesh
    surface_bounding_box = BoundingBox()
    surface_bounding_box.set_model(vertices_info)
    print(surface_bounding_box)
    tets_bounding_box = BoundingBox()
    tets_bounding_box.set_model(vertices_tets)
    print(tets_bounding_box)


    for tet_id, tet in enumerate(tets_quad):
        # query v with inflated bounding box
        queried_vs = query()

        # for each v, update it with tet(if possible)
        for v in queried_vs:
            if v.d_min < 0: continue

            # calc barycentric of v and tet
            b0, b1, b2 = barycentric()

            d = max(-b0, -b1, -b2, -(1 - b0 - b1 - b2))
            if d < v.d_min: 
                # overwrite
                v.d_min = d
                skinning_info[v.id-1].b0 = b0
                skinning_info[v.id-1].b1 = b1
                skinning_info[v.id-1].b2 = b2
                skinning_info[v.id-1].tet_id = tet_id+1


