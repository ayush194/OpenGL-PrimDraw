//
//  default_obj_shader.fs
//  BasicOpenGL
//
//  Created by Ayush Kumar on 7/11/18.
//  Copyright © 2018 Ayush Kumar. All rights reserved.
//

#version 330 core
in vec3 Normal;
in vec3 FragPos;
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

vec3 LightEffect(Light, vec3, vec3);

#define MAX_LIGHTS 100
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_emission1;
uniform bool textured;
uniform int nr_lights;
uniform Material material;
uniform Light lights[MAX_LIGHTS];
uniform vec3 CameraPos;
//while doing light calculations in the model space CameraPos should be enabled

void main() {
    vec3 norm = normalize(Normal);
    vec3 view_dir = normalize(CameraPos - FragPos);
    //Calculate effects of all lights
    vec3 result = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < nr_lights; i++) {
        result += LightEffect(lights[i], norm, view_dir);
    }
    FragColor = vec4(result, 1.0);
    //FragColor = vec4(texture(texture_diffuse1, TexCoords).rgb, 1.0);
}

vec3 LightEffect(Light light, vec3 normal, vec3 view_dir) {
    vec3 light_dir = (light.ltype == 1 ? normalize(-light.direction) : normalize(light.position - FragPos));
    vec3 reflect_dir = reflect(-light_dir, normal);
    
    float attenuation = 1, intensity = 1;
    if (light.ltype == 0 || light.ltype == 2) {
        float distance = length(light.position - FragPos);
        attenuation = 1 / (light.constants[0] * distance + light.constants[1] * distance + light.constants[2] * (distance * distance));
        if (light.ltype == 2) {
            float theta = dot(light_dir, normalize(-light.direction));
            float epsilon = light.inner_cutoff - light.outer_cutoff;
            intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);
        }
    }
    //texture(texture_diffuse1, TexCoords).rgb;
    
    //ambient
    vec3 ambient = light.ambient * (textured ? texture(texture_diffuse1, TexCoords).rgb : material.ambient);
    
    //diffuse
    float diff =max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * light.diffuse * (textured ? texture(texture_diffuse1, TexCoords).rgb : material.diffuse);
    
    //specular
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = spec * light.specular * (textured ? texture(texture_specular1, TexCoords).rgb : material.specular);
    
    //emission
    vec3 emission = texture(texture_emission1, TexCoords).rgb;
    
    //attenuation
    //ambient light shouldn't attenuate
    ambient *= intensity;
    diffuse *= intensity * attenuation;
    specular *= intensity * attenuation;
    emission *= 10 * attenuation;
    
    vec3 result = ambient + diffuse + specular + emission;
    return result;
}
