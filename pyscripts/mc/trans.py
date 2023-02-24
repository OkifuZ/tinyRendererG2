import numpy as np
import pyrender


if __name__ == "__main__":

    verts = []

    with open("mc.txt") as f:
        verts = f.readlines()
        
    vn = len(verts) // 3
    
    with open("out.obj", 'w') as o:
        for line in verts:
            o.write('v ' + line )

        for i in range(vn):
            o.write('f {} {} {}\n'.format(i*3+1, i*3+2, i*3+3))



