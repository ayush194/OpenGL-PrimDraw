//
//  default_light_shader.fs
//  BasicOpenGL
//
//  Created by Ayush Kumar on 7/15/18.
//  Copyright © 2018 Ayush Kumar. All rights reserved.
//

#version 330 core
in vec2 TexCoords;

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    //enum{POINTLIGHT, DIRECTEDLIGHT, SPOTLIGHT}
    int ltype;
    
    vec3 position;
    vec3 direction;
    
    //only for spotlights
    float inner_cutoff;
    float outer_cutoff;
    
    vec3 constants;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_LIGHTS 100
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_emission1;
uniform bool textured;
uniform int nr_lights;
uniform int light_idx;
uniform Material material;
uniform Light lights[MAX_LIGHTS];

void main()
{
    vec3 diffuse = lights[light_idx].ambient * (textured ? texture(texture_diffuse1, TexCoords).rgb : material.diffuse);
    vec3 specular = lights[light_idx].specular * (textured ? texture(texture_specular1, TexCoords).rgb : material.specular);
    vec3 result = diffuse + specular;
    //FragColor = vec4(result, 1.0);
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
