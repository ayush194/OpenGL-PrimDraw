//
//  primitive.h
//  BasicOpenGL
//
//  Created by Ayush Kumar on 7/3/18.
//  Copyright © 2018 Ayush Kumar. All rights reserved.
//

#ifndef primitive_h
#define primitive_h

#include <mesh.h>
#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

#include <cmath>
#include <cassert>
#include <algorithm>

using namespace std;

enum Shape {RECTANGULAR_PLANE, ELLIPTICAL_PLANE, CUBOID, ELLIPSOID, CYLINDER, CONE, TORUS};
enum Normal {ELLIPTICAL_PLANE_NORMAL, ELLIPSOID_NORMAL, CYLINDER_NORMAL, CONE_NORMAL, TORUS_NORMAL};
enum Uvmap {FACE, PROJECT, UNWRAP};

class Primitive : public Mesh {
    typedef unsigned int uint;
public:
    Shape sh;
    float dim[3];
    int subd[2];
    bool smooth;
    explicit Primitive(Shape s, bool sm = false, float a = 1.0, float b = 1.0, float c = 1.0, uint d1 = 1.0, uint d2 = 1.0) : Mesh() {
        switch(s) {
            case RECTANGULAR_PLANE: {
                sh = s; smooth = sm; dim[0] = a; dim[1] = b; dim[2] = 0; subd[0] = 0; subd[1] = -1;
                rectPlane();
                break;
            }
            case ELLIPTICAL_PLANE: {
                sh = s; smooth = sm; dim[0] = a; dim[1] = b; dim[2] = 0; subd[0] = uint(c); subd[1] = -1;
                elliPlane();
                break;
            }
            case CUBOID: {
                sh = s; smooth = sm; dim[0] = a; dim[1] = b; dim[2] = c; subd[0] = 0; subd[1] = 0;
                cuboid();
                break;
            }
            case ELLIPSOID: {
                sh = s; smooth = sm; dim[0] = a; dim[1] = b; dim[2] = c; subd[0] = d1; subd[1] = d2;
                ellipsoid();
                break;
            }
            case CYLINDER: {
                sh = s; smooth = sm; dim[0] = a; dim[1] = b; dim[2] = c; subd[0] = d1; subd[1] = -1;
                cylinder();
                break;
            }
            case CONE: {
                sh = s; smooth = sm; dim[0] = a; dim[1] = b; dim[2] = c; subd[0] = d1; subd[1] = -1;
                cone();
                break;
            }
            case TORUS: {
                sh = s; smooth = sm; dim[0] = a; dim[1] = b; dim[2] = c; subd[0] = d1; subd[1] = d2;
                torus();
                break;
            }
            default: {
                cerr << "This shape is not recognized as a primitive." << endl;
            }
        }
        if (!smooth) {
            facet();
        }
        setupMesh();
    }
    void genUvmap(Uvmap uv) {
        switch (uv) {
            case FACE : {
                assert(!smooth);
                switch (sh) {
                        //for elliptical and rectangular planes use project
                    case ELLIPSOID :
                    case CYLINDER : {
                        unwrapLateralToFace(3 * 4 * (subd[0] + 1), indices.size() - 3 * 4 * (subd[0] + 1));
                    }
                    case CONE : {
                        unwrapCapsToFace();
                        break;
                    }
                    case CUBOID :
                    case TORUS : {
                        unwrapLateralToFace(0, indices.size());
                        break;
                    }
                }
                break;
            }
            case PROJECT : {
                float left_d[2], wid[2];
                switch (sh) {
                    case ELLIPTICAL_PLANE :
                    case ELLIPSOID :
                    case CONE :
                    case CYLINDER : {
                        left_d[0] = -dim[0]; left_d[1] =  -dim[1];
                        wid[0] = 2 * dim[0]; wid[1] = 2 * dim[1];
                        break;
                    }
                    case TORUS : {
                        left_d[0] = -dim[0] - dim[2]; left_d[1] = -dim[1] - dim[2];
                        wid[0] = 2 * (dim[0] + dim[2]); wid[1] = 2 * (dim[1] + dim[2]);
                        break;
                    }
                    case RECTANGULAR_PLANE :
                    case CUBOID : {
                        left_d[0] = -0.5f * dim[0]; left_d[1] = -0.5f * dim[1];
                        wid[0] = dim[0]; wid[1] = dim[1];
                        break;
                    }
                }
                for (uint i = 0; i < vertices.size(); i++) {
                    float x = vertices[i].Position.x, y = vertices[i].Position.y;
                    vertices[i].TexCoords = glm::vec2((x - left_d[0]) / wid[0], (y + 0.5 * dim[1]) / wid[1]);
                }
                break;
            }
            case UNWRAP : {
                switch (sh) {
                    case CUBOID : {
                        //unwrap bottom face
                        uint idx = 0;
                        float piece1[6][2] = {
                            {1.0 / 4, 0.0},
                            {1.0 / 4, 1.0 / 3},
                            {2.0 / 4, 1.0 / 3},
                            {2.0 / 4, 1.0 / 3},
                            {2.0 / 4, 0.0},
                            {1.0 / 4, 0.0},
                        };
                        for (uint i = 0; i < 6; i++) {
                            vertices[indices[idx + i]].TexCoords = glm::vec2(piece1[i][0], piece1[i][1]);
                        }
                        //unwrap lateral faces
                        unwrapLateralFaces(4, 1, 6, 0.0, 1.0, 1.0 / 3, 2.0 / 3);
                        //unwrap top face
                        idx = indices.size() - 6;
                        float piece2[6][2] = {
                            {1.0 / 4, 1.0},
                            {1.0 / 4, 2.0 / 3},
                            {2.0 / 4, 2.0 / 3},
                            {2.0 / 4, 2.0 / 3},
                            {2.0 / 4, 1.0},
                            {1.0 / 4, 1.0},
                        };
                        for (uint i = 0; i < 6; i++) {
                            vertices[indices[idx + i]].TexCoords = glm::vec2(piece2[i][0], piece2[i][1]);
                        }
                        break;
                    }
                    case ELLIPSOID : {
                        uint nxy = 4 * (subd[0] + 1), nz = 2 * (subd[0] + 1);
                        //unwrap bottom cap
                        unwrapBottomCap(nxy, 0, 0.0, 1.0, 0.0, 1.0 / nz);
                        //unwrap lateral faces
                        unwrapLateralFaces(nxy, nz - 2, 3 * nxy, 0.0, 1.0, 1.0 / nz, 1.0 - 1.0 / nz);
                        //unwrap bottom cap
                        unwrapTopCap(nxy, indices.size() - nxy * 3, 0.0, 1.0, 1.0 - 1.0 / nz, 1.0);
                        break;
                    }
                    case CYLINDER : {
                        uint h = 2 * dim[0] + dim[2], nxy = 4 * (subd[0] + 1);
                        //unwrap bottom cap
                        unwrapBottomCap(nxy, 0, 0.0, 1.0, 0.0, dim[0] / h);
                        //unwrap lateral faces
                        unwrapLateralFaces(nxy, 1, 3 * nxy, 0.0, 1.0, dim[0] / h, 1.0 - dim[0] / h);
                        //unwrap bottom cap
                        unwrapTopCap(nxy, indices.size() - 3 * nxy, 0.0, 1.0, 1.0 - dim[0] / h, 1.0);
                        break;
                    }
                    case CONE : {
                        uint h = dim[0] + dim[2], nxy = 4 * (subd[0] + 1);
                        //unwrap bottom cap
                        unwrapBottomCap(nxy, 0, 0.0, 1.0, 0.0, dim[0] / h);
                        //unwrap bottom cap
                        unwrapTopCap(nxy, indices.size() - 3 * nxy, 0.0, 1.0, dim[0] / h, 1.0);
                        break;
                    }
                    case TORUS : {
                        //unwrap lateral faces
                        unwrapLateralFaces(4 * (subd[0] + 1), 4 * (subd[1] + 1));
                        break;
                    }
                }
                break;
            }
        }
        setupMesh();
    }
    void addTexture(uint texid, string textype) {
        Texture* tex = new Texture;
        tex->id = texid;
        tex->type = textype;
        textures.push_back(*tex);
    }
    
private:
    vector<Vertex> buffer_vertices;
    vector<uint> buffer_indices;
    void rectPlane() {
        //creating vertices and normals
        float l[] = {0.5, 0.5, -0.5, -0.5}, w[] = {-0.5, 0.5, 0.5, -0.5};
        for (uint i = 0; i < 4; i++) {
            pushVertex(glm::vec3(dim[0] * l[i], dim[1] * w[i], 0.0), glm::vec3(0.0, 0.0, 1.0));
        }
        //face
        pushFace(0, 1, 2, 3);
    }
    void elliPlane() {
        //creating vertices and normals
        //center vertex
        pushVertex(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
        //lateral vertices
        pushEllipse(dim[0], dim[1], 0.0, subd[0], ELLIPTICAL_PLANE_NORMAL);
        //cap
        pushBottomCap(subd[0]);
        
    }
    void cuboid() {
        //creating vertices and normals
        float l[] = {0.5, 0.5, -0.5, -0.5}, w[] = {-0.5, 0.5, 0.5, -0.5}, h[] = {-0.5, 0.5};
        for (uint i = 0; i < 8; i++) {
            glm::vec3 pos = glm::vec3(dim[0] * l[i % 4], dim[1] * w[i % 4], dim[2] * h[i / 4]);
            pushVertex(pos, glm::normalize(pos));
        }
        //bottom face
        pushFace(0, 1, 2, 3);
        //lateral faces
        pushArrayFaces(0, 4, 2, true, false);
        //top face
        pushFace(4, 5, 6, 7);
    }
    void ellipsoid() {
        //creating vertices and normals
        uint nxy = 4 * (subd[0] + 1), nz = 2 * (subd[1] + 1);
        float pi = acos(-1);
        //bottom vertex
        pushVertex(glm::vec3(0.0, 0.0, -dim[2]), glm::vec3(0.0, 0.0, -1.0));
        //lateral vertices
        for (uint i = 1; i < nz; i++) {
            float thetaz = -pi / 2 + i * pi / nz;
            float z = dim[2] * sin(thetaz);
            float rx = dim[0] * sqrt(1 - (z * z) / (dim[2] * dim[2]));
            float ry = dim[1] * sqrt(1 - (z * z) / (dim[2] * dim[2]));
            pushEllipse(rx, ry, z, subd[0], ELLIPSOID_NORMAL);
        }
        //top vertex
        pushVertex(glm::vec3(0.0, 0.0, dim[2]), glm::vec3(0.0, 0.0, 1));
        //bottom cap
        pushBottomCap(subd[0]);
        //lateral faces
        pushArrayFaces(1, nxy, nz - 1, true, false);
        //top cap
        pushTopCap(subd[0]);
    }
    void cylinder() {
        //bottom vertex
        pushVertex(glm::vec3(0.0, 0.0, -dim[2] / 2), glm::vec3(0.0, 0.0, -1.0));
        //lateral vertices
        pushEllipse(dim[0], dim[1], -dim[2] / 2, subd[0], CYLINDER_NORMAL);
        pushEllipse(dim[0], dim[1], dim[2] / 2, subd[0], CYLINDER_NORMAL);
        //top vertex
        pushVertex(glm::vec3(0.0, 0.0, dim[2] / 2), glm::vec3(0.0, 0.0, 1.0));
        //bottom cap
        pushBottomCap(subd[0]);
        //lateral faces
        uint nxy = 4 * (subd[0] + 1);
        pushArrayFaces(1, nxy, 2, true, false);
        //top cap
        pushTopCap(subd[0]);
    }
    void cone() {
        //bottom vertex
        pushVertex(glm::vec3(0.0, 0.0, -dim[2] / 3), glm::vec3(0.0, 0.0, -1.0));
        //lateral vertices
        pushEllipse(dim[0], dim[1], -dim[2] / 3, subd[0], CONE_NORMAL);
        //top vertex
        pushVertex(glm::vec3(0.0, 0.0, 2 * dim[2] / 3), glm::vec3(0.0, 0.0, 1.0));
        //bottom cap
        pushBottomCap(subd[0]);
        //top cap
        pushTopCap(subd[0]);
    }
    void torus() {
        uint nxy = 4 * (subd[0] + 1), nz = 4 * (subd[1] + 1);
        float pi = acos(-1);
        for (uint i = 0; i < nz; i++) {
            float theta = 2 * i * pi / nz, t = cos(theta), u = sin(theta);
            pushEllipse(dim[0] - dim[2] * t, dim[1] - dim[2] * t, dim[2] * u, subd[0], TORUS_NORMAL);
        }
        //FACES
        pushArrayFaces(0, nxy, nz, true, true);
    }

    void facet() {
        swap(vertices, buffer_vertices);
        swap(indices, buffer_indices);
        vertices.clear();
        indices.clear();
        for (uint i = 0; i < buffer_indices.size(); i += 3) {
            glm::vec3 p1 = buffer_vertices[buffer_indices[i]].Position;
            glm::vec3 p2 = buffer_vertices[buffer_indices[i + 1]].Position;
            glm::vec3 p3 = buffer_vertices[buffer_indices[i + 2]].Position;
            glm::vec3 n = cross(p2 - p1, p3 - p2);
            if (glm::dot(n, p1) < 0) { n = -n; }
            pushVertex(p1, n);
            pushVertex(p2, n);
            pushVertex(p3, n);
            indices.insert(indices.end(), {i, i + 1, i + 2});
        }
    }
    
    
    //HELPER FUNCTIONS
    void unwrapLateralFaces(uint nxy, uint nz, uint idx = 0, float tex_xi = 0.0, float tex_xf = 1.0, float tex_yi = 0.0, float tex_yf = 1.0) {
        //assert(0.0 <= tex_xi <= 1.0);
        float w = (tex_xf - tex_xi) / nxy, h = (tex_yf - tex_yi) / nz;
        for (float i = 0; i < nz; i++) {
            for (float j = 0; j < nxy; j++) {
                float piece[6][2] = {
                    {tex_xi + j * w ,         tex_yi + i * h},
                    {tex_xi + j * w,          tex_yi + (i + 1) * h},
                    {tex_xi + (j + 1) * w,    tex_yi + (i + 1) * h},
                    {tex_xi + (j + 1) * w,    tex_yi + (i + 1) * h},
                    {tex_xi + (j + 1) * w,    tex_yi + i * h},
                    {tex_xi + j * w,          tex_yi + i * h},
                };
                for (uint k = 0; k < 6; k++) {
                    vertices[indices[idx + k]].TexCoords = glm::vec2(piece[k][0], piece[k][1]);
                }
                idx += 6;
            }
        }
    }
    
    void unwrapBottomCap(uint nxy, uint idx = 0, float tex_xi = 0.0, float tex_xf = 1.0, float tex_yi = 0.0, float tex_yf = 1.0) {
        float w = (tex_xf - tex_xi) / nxy, h = (tex_yf - tex_yi);
        for (uint i = 0; i < nxy; i++) {
            float piece[3][2] = {
                {tex_xi + i * w,            tex_yi + h},
                {tex_xi + (i + 0.5f) * w,    tex_yi},
                {tex_xi + (i + 1) * w,      tex_yi + h}
            };
            for (uint k = 0; k < 3; k++) {
                vertices[indices[idx + k]].TexCoords = glm::vec2(piece[k][0], piece[k][1]);
            }
            idx += 3;
        }
    }
    
    void unwrapTopCap(uint nxy, uint idx = 0, float tex_xi = 0.0, float tex_xf = 1.0, float tex_yi = 0.0, float tex_yf = 1.0) {
        float w = (tex_xf - tex_xi) / nxy, h = (tex_yf - tex_yi);
        for (uint i = 0; i < nxy; i++) {
            float piece[3][2] = {
                {tex_xi + (i + 1) * w,      tex_yi},
                {tex_xi + (i + 0.5f) * w,    tex_yi + h},
                {tex_xi + (i) * w,          tex_yi}
            };
            for (uint k = 0; k < 3; k++) {
                vertices[indices[idx + k]].TexCoords = glm::vec2(piece[k][0], piece[k][1]);
            }
            idx -= 3;
        }
    }
    
    void unwrapCapsToFace() {
        uint nxy = 4 * (subd[0] + 1);
        float piece1[3][2] = {{0.0, 1.0}, {0.5, 0.0}, {1.0, 1.0}};
        for (uint idx = 0; idx < 3 * nxy; idx += 3) {
            for (uint i = 0; i < 3; i++) {
                vertices[indices[idx + i]].TexCoords = glm::vec2(piece1[i][0], piece1[i][1]);
            }
        }
        float piece2[3][2] = {{1.0, 0.0}, {0.0, 0.5}, {0.0, 0.0}};
        for (uint idx = indices.size() - 3 * nxy; idx < indices.size(); idx += 3) {
            for (uint i = 0; i < 3; i++) {
                vertices[indices[idx + i]].TexCoords = glm::vec2(piece2[i][0], piece2[i][1]);
            }
        }
    }
    
    void unwrapLateralToFace(uint a, uint b) {
        for (uint idx = a; idx < b; idx += 6) {
            float piece[6][2] = {{1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}, {0.0, 0.0}, {1.0, 0.0}};
            for (uint i = 0; i < 6; i++) {
                vertices[indices[idx + i]].TexCoords = glm::vec2(piece[i][0], piece[i][1]);
            }
        }
    }
    
    void pushEllipse(float a, float b, float z, uint d, Normal n) {
        int nxy = 4 * (d + 1);
        for (uint i = 0; i < nxy; i++) {
            pushVertex(getEllipticCoord(a, b, z, i, nxy), normals(a, b, z, i, nxy, n));
        }
    }
    
    void pushVertex(glm::vec3 p, glm::vec3 n) {
        Vertex* vert = new Vertex;
        vert->Position = p;
        vert->Normal = glm::normalize(n);
        vertices.push_back(*vert);
    }
    
    glm::vec3 normals(float a, float b, float z, uint i, uint nxy, Normal n) {
        switch(n) {
            case TORUS_NORMAL : return glm::normalize(getEllipticCoord(a, b, z, i, nxy) - getEllipticCoord(dim[0], dim[1], 0, i, nxy));
            case ELLIPSOID_NORMAL : return glm::normalize(getEllipticCoord(a, b, z, i, nxy));
            case ELLIPTICAL_PLANE_NORMAL : return glm::vec3(0.0, 0.0, 1.0);
            case CYLINDER_NORMAL : return glm::normalize(glm::vec3(0.0, 0.0, z) - glm::vec3(0.0, 0.0, 0.0));
            case CONE_NORMAL : return glm::normalize(getEllipticCoord(a, b, z, i, nxy) - glm::vec3(0.0, 0.0, 0.0));
        }
        return glm::vec3(0.0, 0.0, 1.0);
    }
    
    glm::vec3 getEllipticCoord(float a, float b, float z, uint i, uint nxy) {
        float pi = acos(-1), x, y;
        if (i == nxy / 4 || i == 3 * nxy / 4) {
            //to handle overflows at theta = pi/2 && theta = 3pi/2
            x = 0.0; y = (i == nxy / 4 ? b : -b);
        } else {
            float theta = 2 * i * pi / nxy, m = tan(theta);
            x = 1 / sqrt(1 / (a * a) + (double(m) * m) / (b * b));
            if (pi / 2 < theta && theta < 3 * pi / 2) { x = -x; }
            y = m * x;
        }
        return glm::vec3(x, y, z);
    }
    
    void pushArrayFaces(uint vbegin, uint nxy, uint nz, bool wrapxy, bool wrapz) {
        for (uint i = 0; i < nz; i++) {
            if (i == nz - 1 && !wrapz) { continue; }
            for (uint j = 0; j < nxy; j++) {
                if (j == nxy - 1 && !wrapxy) { continue; }
                //quad with vertices a b c d
                uint a = vbegin + nxy * i + j;
                uint b = vbegin + nxy * ((i + 1) % nz) + j;
                uint c = vbegin + nxy * ((i + 1) % nz) + (j + 1) % nxy;
                uint d = vbegin + nxy * i + (j + 1) % nxy;
                pushFace(a, b, c, d);
            }
        }
    }
    
    void pushFace(uint a, uint b, uint c, uint d) {
        indices.insert(indices.end(), {a, b, c});
        indices.insert(indices.end(), {c, d, a});
    }
    
    void pushTopCap(uint d) {
        uint vcount = vertices.size();
        uint nxy = 4 * (d + 1);
        /*
        for (int i = 1; i <= nxy; i++) {
            indices.insert(indices.end(), {vcount - (i + 1), vcount - 1, vcount - i % nxy - 2});
        }
        */
        for (uint i = vcount - nxy - 1; i < vcount - 1; i++) {
            indices.insert(indices.end(), {vcount - nxy - 1 + (i + 1 - vcount + nxy + 1) % nxy, vcount - 1, i});
        }
    }
    void pushBottomCap(uint d) {
        uint nxy = 4 * (d + 1);
        for (uint i = 1; i <= nxy; i++) {
            indices.insert(indices.end(), {i, 0, 1 + i % nxy});
        }
    }
};

#endif /* primitive_h */
