//
//  scene.h
//  BasicOpenGL
//
//  Created by Ayush Kumar on 7/6/18.
//  Copyright © 2018 Ayush Kumar. All rights reserved.
//

#ifndef scene_h
#define scene_h

#include <map>
#include <string>
#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <cstdlib>
#include <camera.h>
#include <mesh.h>
#include <primitive.h>
#include <shader.h>
#include <material.h>
#include <texture.h>

extern Camera CAMERA;
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

enum LightType {POINTLIGHT, DIRECTEDLIGHT, SPOTLIGHT};

class Object {
 public:
    Object() {}
    Object(Primitive* pm, glm::mat4& t, glm::mat4& r, glm::mat4& s, Material* mat, Shader* sh, bool tex, bool isl) :
    base_mesh(pm), translate(t), rotate(r),  scale(s), material(mat), shader(sh), textured(tex), islight(isl) {}
    void Draw() {
        //create transform matrices
        glm::mat4 view, projection, model;
        view = CAMERA.GetViewMatrix();
        projection = glm::perspective(glm::radians(CAMERA.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        //needs to be in reverse order since glm stores matrices columnwise
        model = translate * rotate * scale;
        shader->use();
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
        shader->setMat4("model", model);
        shader->setBool("textured", textured);
        shader->setVec3("material.ambient", material->ambient);
        shader->setVec3("material.diffuse", material->diffuse);
        shader->setVec3("material.specular", material->specular);
        shader->setFloat("material.shininess", material->shininess);
        base_mesh->Draw(*shader);
    }
    Primitive* base_mesh;
    glm::mat4 translate;
    glm::mat4 rotate;
    glm::mat4 scale;
    Material* material;
    Shader* shader;
    bool textured;
    bool islight;
};

class Light {
public:
    Light() {}
    Light(LightType l, Object* lobj, glm::vec3 pos, glm::vec3 dir, float incut, float outcut, glm::vec3 c, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec) :
    ltype(l), lightobject(lobj), position(pos), direction(dir), inner_cutoff(incut), outer_cutoff(outcut), constants(c), ambient(amb), diffuse(diff), specular(spec) {}
    void Draw() {
        //bool tmp = lightobject->renderable;
        //lightobject->renderable = true;
        lightobject->Draw();
        //lightobject->renderable = tmp;
    }
    LightType ltype;
    
    Object* lightobject;
    
    glm::vec3 position;
    glm::vec3 direction;
    
    //only for SpotLights
    float inner_cutoff;
    float outer_cutoff;
    
    //constant, linear, quadratic constants for attenuation
    glm::vec3 constants;
    
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class Scene {
    typedef unsigned int uint;
public:
    map<uint, Light> lights;
    map<uint, Object> objects;
    map<uint, Material> materials;
    map<uint, Shader> shaders;
    vector<uint> textures;
    const static pair<string, string> default_obj_shader;
    const static pair<string, string> default_light_shader;
    const static glm::vec3 def;
    Scene() {
        dim[0] = dim[1] = dim[2] = 1.0;
        subd[0] = subd[1] = 5;
        smooth = false;
        //default shader and material
        createShader(default_obj_shader.first, default_obj_shader.second);
        createShader(default_light_shader.first, default_light_shader.second);
        createMaterial(CHROME);
    }
    ~Scene() {
        lights.clear();
        objects.clear();
        materials.clear();
        shaders.clear();
    }
    void setDim(float a, float b, float c) {
        dim[0] = a; dim[1] = b; dim[2] = c;
    }
    void setSubd(int d1, int d2) {
        subd[0] = d1; subd[1] = d2;
    }
    void setSmooth(bool sm) {
        smooth = sm;
    }
    void setLightCutoff(int incut, int outcut) {
        light_inner_cutoff = incut;
        light_outer_cutoff = outcut;
    }
    void setLightConstants(glm::vec3 c) {
        light_constants = c;
    }
    void setLightAmbient(glm::vec3 amb) {
        light_ambient = amb;
    }
    void setLightDiffuse(glm::vec3 diff) {
        light_diffuse = diff;
    }
    void setLightSpecular(glm::vec3 spec) {
        light_specular = spec;
    }
    void setRenderLights(bool val) {
        render_lights = val;
    }
    /*
    void scatter(Shape s, int n, int axes = 3, float scale_jitter = 0, float rotate_jitter = 0, int shaderid = 0, int materialid = 0) {
        for (uint i = 0; i < n; i++) {
            //randomise based on jitters
            createObject(s, translate, scale, rotate, deg, shaderid, materialid);
        }
    }
     */
    uint createObject(Shape sh, glm::vec3 t = def, glm::vec3 r = def, glm::vec3 s = def, float deg = 0, bool isl = false, uint shaderid = 0, uint materialid = 0) {
        uint objectid = objects.size() ? objects.rbegin()->first + 1 : 0;
        Primitive* primitive = new Primitive(sh, smooth, dim[0], dim[1], dim[2], subd[0], subd[1]);
        glm::mat4 identity;
        glm::mat4 translate = glm::translate(identity, t);
        glm::mat4 rotate = glm::rotate(identity, glm::radians(deg), r);
        glm::mat4 scale = glm::scale(identity, s);
        Shader* shader = &(shaders[shaderid]);
        Material* material = &(materials[materialid]);
        objects[objectid] = Object(primitive, translate, rotate, scale, material, shader, false, isl);
        return objectid;
    }
    uint createLight(LightType ltype, uint objid, glm::vec3 pos = def, glm::vec3 dir = def) {
        uint lightid = lights.size() ? lights.rbegin()->first + 1 : 0;
        lights[lightid] = Light(ltype, &objects[objid], pos, dir, light_inner_cutoff, light_outer_cutoff, light_constants, light_ambient, light_diffuse, light_specular);
        return lightid;
    }
    uint createMaterial(glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float s) {
        uint materialid = materials.size() ? materials.rbegin()->first + 1 : 0;
        materials[materialid] = Material(amb, diff, spec, s);
        return materialid;
    }
    uint createShader(string vs, string fs) {
        uint shaderid = shaders.size() ? shaders.rbegin()->first + 1 : 0;
        shaders[shaderid] = Shader(vs.c_str(), fs.c_str());
        return shaderid;
    }
    uint createTexture(string path) {
        uint textureid = loadTexture(path.c_str());
        textures.push_back(textureid);
        return textureid;
    }
    void setTexture(TEXTURETYPE textype, uint obj, uint texid) {
        switch (textype) {
            case DIFFUSE : {objects[obj].base_mesh->addTexture(texid, "texture_diffuse"); break;}
            case SPECULAR : {objects[obj].base_mesh->addTexture(texid, "texture_specular"); break;}
            case EMISSION : {objects[obj].base_mesh->addTexture(texid, "texture_emission"); break;}
            case NORMAL : {objects[obj].base_mesh->addTexture(texid, "texture_normal"); break;}
            case HEIGHT : {objects[obj].base_mesh->addTexture(texid, "texture_height"); break;}
        }
        objects[obj].textured = true;
    }
    void setUvmap(uint obj, Uvmap m) {
        objects[obj].base_mesh->genUvmap(m);
    }
    void setMaterial(uint obj, uint materialid) {
        objects[obj].material = &materials[materialid];
    }
    void deleteLight(uint lightid) {
        lights.erase(lightid);
    }
    void deleteObject(uint objectid) {
        objects.erase(objectid);
    }
    void deleteMaterial(uint materialid) {
        materials.erase(materialid);
    }
    void deleteShader(uint shaderid) {
        shaders.erase(shaderid);
    }
    void deleteTexture(uint textureid) {
        glDeleteTextures(1, &textureid);
    }
    void render() {
        //bind global variables to all shaders
        for (auto it = shaders.begin(); it != shaders.end(); it++) {
            it->second.use();
            it->second.setVec3("CameraPos", CAMERA.Position);
            it->second.setInt("nr_lights", lights.size());
            uint i = 0;
            for (auto it2 = lights.begin(); it2 != lights.end(); it2++) {
                //send light's data as uniform to the new_shader
                it->second.setInt("lights[" + to_string(i) + "].ltype", int(it2->second.ltype));
                it->second.setVec3("lights[" + to_string(i) + "].position", it2->second.position);
                it->second.setVec3("lights[" + to_string(i) + "].direction", it2->second.direction);
                it->second.setFloat("lights[" + to_string(i) + "].inner_cutoff", it2->second.inner_cutoff);
                it->second.setFloat("lights[" + to_string(i) + "].outer_cutoff", it2->second.outer_cutoff);
                it->second.setVec3("lights[" + to_string(i) + "].constants", it2->second.constants);
                it->second.setVec3("lights[" + to_string(i) + "].ambient", it2->second.ambient);
                it->second.setVec3("lights[" + to_string(i) + "].diffuse", it2->second.diffuse);
                it->second.setVec3("lights[" + to_string(i) + "].specular", it2->second.specular);
                i++;
            }
        }
        for (auto it = objects.begin(); it != objects.end(); it++) {
            if (!it->second.islight) {it->second.Draw();}
        }
        if (render_lights) {
            uint i = 0;
            for (auto it = lights.begin(); it != lights.end(); it++) {
                it->second.lightobject->shader->setInt("light_idx", i);
                it->second.Draw();
                i++;
            }
        }
    }
    
private:
    float dim[3];
    int subd[2];
    bool smooth;
    float light_inner_cutoff;
    float light_outer_cutoff;
    glm::vec3 light_constants;
    glm::vec3 light_ambient;
    glm::vec3 light_diffuse;
    glm::vec3 light_specular;
    bool render_lights;
};

const pair<string, string> Scene::default_obj_shader = make_pair("shaders/default_obj_shader.vs", "shaders/default_obj_shader.fs");
const pair<string, string> Scene::default_light_shader = make_pair("shaders/default_light_shader.vs", "shaders/default_light_shader.fs");
const glm::vec3 Scene::def = glm::vec3(1.0, 1.0, 1.0);
#endif /* scene_h */
