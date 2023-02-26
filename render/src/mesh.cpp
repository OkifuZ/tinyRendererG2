#include "mesh.h"
#include "file_system.h"
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <set>
#include <unordered_set>


MeshDataContainer::MeshDataContainer(std::vector<float>& verts, std::vector<float>& texts, std::vector<float>& norms,
    std::vector<uint32_t>& faces_vertID, std::vector<uint32_t>& faces_textID, std::vector<uint32_t>& faces_normID,
    std::vector<uint32_t>& tets_vertID,
    std::string& name_in_objfile) :
    verts(verts), texts(texts), norms(norms),
    faces_vertID(faces_vertID), faces_textID(faces_textID), faces_normID(faces_normID), tets_vertID(tets_vertID),
    name_in_objfile(name_in_objfile)
{
    verts_num = verts.size() / 3;
    face_num = faces_vertID.size() / 3;
    tet_num = tets_vertID.size() / 4;
}


void rearrange_meshdata(MeshDataContainer_ptr);
void set_edges(MeshDataContainer_ptr mesh);

bool save_mesh(MeshDataContainer_ptr mesh, const std::string& fpath) {
    try {
        std::ofstream of(fpath, std::ios::trunc);
        if (of.is_open()) {
            of << "# " << "vert num: " << mesh->verts_num << ", face num: " << mesh->face_num << "\n";
            of << "o " << mesh->name_in_objfile << "\n";
            for (size_t i = 0; i < mesh->verts_num; i++) {
                of << "v "
                    << mesh->verts[i * 3 + 0] << " "
                    << mesh->verts[i * 3 + 1] << " "
                    << mesh->verts[i * 3 + 2] << "\n";
            }
            for (size_t i = 0; i < mesh->verts_num; i++) {
                of << "vn "
                    << mesh->norms[i * 3 + 0] << " "
                    << mesh->norms[i * 3 + 1] << " "
                    << mesh->norms[i * 3 + 2] << "\n";
            }
            of << "s 1\n";
            for (size_t i = 0; i < mesh->face_num; i++) {
                of << "f "
                    << mesh->faces_vertID[i * 3 + 0] << "\\\\" << mesh->faces_vertID[i * 3 + 0] << " "
                    << mesh->faces_vertID[i * 3 + 1] << "\\\\" << mesh->faces_vertID[i * 3 + 1] << " "
                    << mesh->faces_vertID[i * 3 + 2] << "\\\\" << mesh->faces_vertID[i * 3 + 2] << "\n";
            }
            for (size_t i = 0; i < mesh->tet_num; i++) {
                of << "tet "
                    << mesh->tets_vertID[i * 4 + 0] << " "
                    << mesh->tets_vertID[i * 4 + 1] << " "
                    << mesh->tets_vertID[i * 4 + 2] << " "
                    << mesh->tets_vertID[i * 4 + 3] << "\n";
            }
        }
        else return false;
    }
    catch (...) {
        return false;
    }

}

void set_edges(MeshDataContainer_ptr mesh) {
    using edge_type = std::tuple<size_t, size_t>;
    auto my_hash = [](const edge_type& e) {
        auto& [e1, e2] = e;
        return std::hash<float>{}(e1) / 2 + std::hash<float>{}(e2) / 2;
    };
    
    std::unordered_set<edge_type, decltype(my_hash)> edges_set(0, my_hash);
    
    const auto& face_ids = mesh->faces_vertID;
    size_t e1{}, e2{};
    for (size_t i = 0; i < mesh->face_num; i++) {
        e1 = mesh->faces_vertID[i * 3 + 0];
        e2 = mesh->faces_vertID[i * 3 + 1];
        if (e1 > e2) std::swap(e1, e2);
        edges_set.insert(std::tuple{ e1, e2 });
        e1 = mesh->faces_vertID[i * 3 + 1];
        e2 = mesh->faces_vertID[i * 3 + 2];
        if (e1 > e2) std::swap(e1, e2);
        edges_set.insert(std::tuple{ e1, e2 });
        e1 = mesh->faces_vertID[i * 3 + 0];
        e2 = mesh->faces_vertID[i * 3 + 2];
        if (e1 > e2) std::swap(e1, e2);
        edges_set.insert(std::tuple{ e1, e2 });
    }
    std::vector <uint32_t> edges; edges.reserve(2 * edges_set.size());
    for (auto& e : edges_set) {
        auto& [e1, e2] = e;
        edges.push_back(e1);
        edges.push_back(e2);
    }

    mesh->edges_vertID = edges;
    mesh->edge_num = edges.size() / 2;
}

void load_mesh(MeshDataContainer_ptr mesh, const std::string& fpath) {
    const static int max_loop_cnt = 200000;
    int loop_cnt = 0;

    std::ifstream inf(fpath);
    if (!inf.is_open()) {
        printf("[load_mesh] ERROR: can not open file %s\n", fpath.c_str());
        exit(-1);
    }
    std::string line;

    std::vector<float> verts;
    std::vector<float> texts;
    std::vector<float> norms;
    std::vector<uint32_t> faces_vertID;
    std::vector<uint32_t> faces_textID;
    std::vector<uint32_t> faces_normID;
    std::vector<uint32_t> tet_vertID;
    std::string objname = "";

    std::string head = "";
    float x, y, z;
    uint32_t a1 = 0, b1 = 0, c1 = 0;
    uint32_t a2 = 0, b2 = 0, c2 = 0;
    uint32_t a3 = 0, b3 = 0, c3 = 0;
    uint32_t tet1{}, tet2{}, tet3{}, tet4{};
    char split;

    size_t vert_num = 0;
    size_t face_num = 0;
    size_t uv_num = 0;
    size_t norm_num = 0;
    size_t tet_num = 0;

    while (std::getline(inf, line)) {
        if (loop_cnt > max_loop_cnt) {
            printf("[load_mesh] ERROR: max_loop_cnt reached\n");
            exit(-1);
        }
        std::istringstream isf(line);

        if (!(isf >> head)) continue;
        if (head.size() <= 0) continue;
        else if (head[0] == '#') continue;
        else if (head == "o") isf >> objname;
        else if (head == "v") {
            if (!(isf >> x >> y >> z)) continue;
            vert_num++;
            verts.insert(verts.end(), { x, y, z });
        }
        else if (head == "vt") {
            if (!(isf >> x >> y >> z)) continue;
            uv_num++;
            texts.insert(texts.end(), { x, y });
        }
        else if (head == "vn") {
            if (!(isf >> x >> y >> z)) continue;
            norm_num++;
            norms.insert(norms.end(), { x, y, z });
        }
        else if (head == "f") {
            if (line.find("//") != std::string::npos) { // f v//n
                if (!(isf >> a1 >> split >> split >> c1)) continue;
                b1 = 0;
                if (!(isf >> a2 >> split >> split >> c2)) continue; 
                b2 = 0;
                if (!(isf >> a3 >> split >> split >> c3)) continue; 
                b3 = 0;
                faces_vertID.insert(faces_vertID.end(), { a1, a2, a3 });
                faces_normID.insert(faces_normID.end(), { c1, c2, c3 });
            }
            else if (line.find("/") != std::string::npos) { // f v/t/n
                if (!(isf >> a1 >> split >> b1 >> split >> c1)) continue;
                if (!(isf >> a2 >> split >> b2 >> split >> c2)) continue;
                if (!(isf >> a3 >> split >> b3 >> split >> c3)) continue;
                faces_vertID.insert(faces_vertID.end(), { a1, a2, a3 });
                faces_textID.insert(faces_textID.end(), { b1, b2, b2 });
                faces_normID.insert(faces_normID.end(), { c1, c2, c3 });
            }
            else { // f v
                if (!(isf >> a1)) continue; b1 = c1 = 0;
                if (!(isf >> a2)) continue; b2 = c2 = 0;
                if (!(isf >> a3)) continue; b3 = c3 = 0;
                faces_vertID.insert(faces_vertID.end(), { a1, a2, a3 });
            }
            face_num++;
        }
        else if (head == "tet") {
            if (!(isf >> tet1 >> tet2 >> tet3 >> tet4)) continue;
            tet_vertID.insert(tet_vertID.end(), { tet1, tet2, tet3, tet4 });
            tet_num++;
        }
        loop_cnt++;
    }

    // tackle 0-started/1-started problem
    bool has_zero_vertID = false;
    for (auto& id : faces_vertID) has_zero_vertID = (id == 0);
    if (!has_zero_vertID)
        for (auto& id : faces_vertID) id -= 1;

    bool has_zero_normID = false;
    for (auto& id : faces_normID) has_zero_normID = (id == 0);
    if (!has_zero_normID)
        for (auto& id : faces_normID) id -= 1;

    bool has_zero_textID = false;
    for (auto& id : faces_textID) has_zero_textID = (id == 0);
    if (!has_zero_textID)
        for (auto& id : faces_textID) id -= 1;

    bool has_zero_tetID = false;
    for (auto& id : tet_vertID) has_zero_tetID = (id == 0);
    if (!has_zero_tetID)
        for (auto& id : tet_vertID) id -= 1;

    mesh->faces_normID = faces_normID;
    mesh->faces_textID = faces_textID;
    mesh->faces_vertID = faces_vertID;
    mesh->tets_vertID = tet_vertID;
    mesh->name_in_objfile = objname;
    mesh->face_num = face_num;
    mesh->verts_num = vert_num;
    mesh->uv_num = uv_num;
    mesh->tet_num = tet_num;
    mesh->norm_num = norm_num;
    mesh->verts = verts;
    mesh->norms = norms;
    mesh->texts = texts;
    rearrange_meshdata(mesh);

    set_edges(mesh);

    printf("load_mesh() %s completed\n", fpath.c_str());
}


void rearrange_meshdata(MeshDataContainer_ptr mesh) {
    if (mesh == nullptr) {
        printf("Error: mesh_to_VAO(), mullptr\n");
        exit(-1);
    }

    bool has_normal_ids = (mesh->faces_normID.size() != 0);
    bool has_texture_ids = (mesh->faces_textID.size() != 0);
    bool has_normal = (mesh->norms.size() != 0);
    bool has_texture = (mesh->texts.size() != 0);


    if ((mesh->verts.size() != mesh->verts_num * 3) ||
        (has_normal && mesh->norms.size() != mesh->verts_num * 3) ||
        (has_texture && mesh->texts.size() != mesh->verts_num * 3) ||
        (mesh->faces_vertID.size() != mesh->face_num * 3) ||
        (has_normal_ids && mesh->faces_normID.size() != mesh->face_num * 3) ||
        (has_texture_ids && mesh->faces_textID.size() != mesh->face_num * 3))
    {
        printf("Error: conflict vector size in mesh data - rearrange_meshdata()\n");
        exit(-1);
    }

    if (!has_normal_ids && !has_texture_ids) return;

    std::vector<float> verts = mesh->verts;
    std::vector<float> norms(has_normal ? mesh->verts_num * 3 : 0, std::numeric_limits<float>::max());
    std::vector<float> texts(has_texture ? mesh->verts_num * 2 : 0, std::numeric_limits<float>::max());

    std::set<uint32_t> visited;

    if (has_normal_ids || has_texture_ids) {
        for (auto i = 0; i < mesh->face_num * 3; i++) {
            auto vi = mesh->faces_vertID[i];
            if (visited.count(vi) == 0) {
                visited.insert(vi);
                if (has_normal_ids && has_normal) {
                    auto ni = mesh->faces_normID[i]; // (vi, ni)
                    norms[vi * 3 + 0] = mesh->norms[ni * 3 + 0];
                    norms[vi * 3 + 1] = mesh->norms[ni * 3 + 1];
                    norms[vi * 3 + 2] = mesh->norms[ni * 3 + 2];
                }
                if (has_texture_ids && has_texture) {
                    auto ti = mesh->faces_textID[i]; // (vi, ti)
                    texts[vi * 2 + 0] = mesh->texts[ti * 2 + 0];
                    texts[vi * 2 + 1] = mesh->texts[ti * 2 + 1];
                }
            }
        }
    }

    for (auto& norm : norms) {
        if (norm == std::numeric_limits<float>::max()) {
            // ERROR: TODO
            printf("Error: norms failed in mesh data - rearrange_meshdata()\n");
            exit(-1);
        }
    }
    for (auto& text : texts) {
        if (text == std::numeric_limits<float>::max()) {
            printf("Error: texts failed in mesh data - rearrange_meshdata()\n");
            exit(-1);
        }
    }

    if (has_normal && has_normal_ids) {
        mesh->norms = norms;
        mesh->faces_normID.clear();
    }
    if (has_texture && has_texture_ids) {
        mesh->texts = texts;
        mesh->faces_textID.clear();
    }

    if (has_normal) {
        int cnt = 0;
        for (size_t i = 0; i < mesh->face_num; i++) {
            auto v1 = mesh->faces_vertID[i * 3 + 0];
            glm::vec3 face_norm{mesh->norms[v1 * 3 + 0], mesh->norms[v1 * 3 + 1] , mesh->norms[v1 * 3 + 2]};
            auto v2 = mesh->faces_vertID[i * 3 + 1];
            face_norm += glm::vec3{ mesh->norms[v2 * 3 + 0], mesh->norms[v2 * 3 + 1] , mesh->norms[v2 * 3 + 2] };
            auto v3 = mesh->faces_vertID[i * 3 + 2];
            face_norm += glm::vec3{ mesh->norms[v3 * 3 + 0], mesh->norms[v3 * 3 + 1] , mesh->norms[v3 * 3 + 2] };
            face_norm = glm::normalize(face_norm);

            glm::vec3 vert1 = { mesh->verts[v1 * 3 + 0], mesh->verts[v1 * 3 + 1], mesh->verts[v1 * 3 + 2] };
            glm::vec3 vert2 = { mesh->verts[v2 * 3 + 0], mesh->verts[v2 * 3 + 1], mesh->verts[v2 * 3 + 2] };
            glm::vec3 vert3 = { mesh->verts[v3 * 3 + 0], mesh->verts[v3 * 3 + 1], mesh->verts[v3 * 3 + 2] };
            glm::vec3 norm_ccw = glm::normalize(glm::cross((vert2 - vert1), (vert3 - vert2)));

            if (glm::dot(norm_ccw, face_norm) < 0) cnt++;
        }

        if (cnt > 0) {
            printf("[rearrange_meshdata()] Winding order conflict with normal\n");
        }
    }

}


MeshDataContainer_ptr generate_lines_mesh(std::vector<std::tuple<glm::vec3, glm::vec3>> lines) {
    std::vector<float> verts;
    verts.reserve(6 * lines.size());

    std::vector<uint32_t> faces_vertID;
    faces_vertID.reserve(2 * lines.size());

    uint32_t idx = 0;
    for (auto& seg : lines) {
        auto& [begin, end] = seg;
        verts.insert(verts.begin(), { begin.x, begin.y, begin.z, end.x, end.y, end.z });
        faces_vertID.insert(faces_vertID.begin(), { idx, idx + 1 });
        idx += 2;
    }
    auto& res = std::make_shared<MeshDataContainer>();
    res->faces_vertID = faces_vertID;
    res->verts = verts;
    res->name_in_objfile = "generated";

    return res;
}