from tets.utils import read_objfile, wirte_objfile


height:int = 20
width:int = 20
square_h:float = 3.0 / height
square_w:float = 3.0 / width


if __name__ == "__main__":

    verts = []
    face = []

    for i in range(height + 1):
        for j in range(width + 1):
            h = i - height / 2
            w = j - width / 2
            verts.append([h * square_h, 0, w * square_w])

    for i in range(height):
        for j in range(width):
            i1 = i * (width + 1) + j + 1
            i2 = i1 + 1
            i3 = (i + 1) * (width + 1) + j + 1
            i4 = i3 + 1
            face.append([i1, i3, i2])
            face.append([i2, i3, i4])

    wirte_objfile('cloth_{}x{}'.format(height, width), 
    **{
        'vertices': verts, 
        'triangles':face,
    })
