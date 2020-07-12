#ifndef OBJPARSER_H
#define OBJPARSER_H

#include <string>
#include <vector>
#include <map>

#include <vecmath.h>

struct material {
    float shininess;
    Vector3f ambient;
    Vector3f diffuse; // ignored if there is diffuse texture
    Vector3f specular;
    std::string diffuse_texture; // 纹理是一个名称
};

// data is an array of RGB pixels.
// there is no alpha channel.
struct rgbimage {  // 从纹理图片中解析出来的RGB图片
    int w;
    int h;
    std::vector<uint8_t> data;
};

// a single obj file can contain multiple pieces of geometry.
// we refer to each piece as "batch", since it is drawn in a single draw call.      这些batch里面的图元是在一个draw call中绘制的
// All triangles in a batch have the same material parameters.      所有在一个batch里面的三角的都拥有一样的材质参数
struct draw_batch {
    std::string name; // useful for debugging
    int start_index; // 指向scene.indices的索引
    int nindices;
    material mat;
};

class objparser {
public:
    // return false on error
    bool parse(const std::string& objfile);
    void clear();

    // the parse() method fills these arrays with vertex, index, and texture data
    std::vector<Vector3f>           positions;
    std::vector<Vector3f>           normals;
    std::vector<Vector2f>           texcoords;

    std::vector<uint32_t>           indices;
    std::vector<draw_batch>         batches;  // 分batches存储的原因 应该是减少draw call的调用次数
    std::map<std::string, rgbimage> textures;  // 纹理名映射到rgb图片数据的map

private:
    // parse materials from .mtl file and store in materials map.
    bool parsemtl(const std::string& mtlfile, 
                  std::map<std::string, material> * materials);

    // parse textures referenced by mtl file.
    bool loadtextures(const std::string& basepath, 
                      const std::map<std::string, material>& materials);
};

#endif