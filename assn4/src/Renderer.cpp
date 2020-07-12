#include "Renderer.h"

#include "ArgParser.h"
#include "Camera.h"
#include "Image.h"
#include "Ray.h"
#include "VecUtils.h"
#include "RenderUtils.h"
#include <limits>

Renderer::Renderer(const ArgParser &args) :
    _args(args),
    _scene(args.input_file) {
}

void
Renderer::Render() {
    // 图片的高宽
    int w = _args.width;
    int h = _args.height;

    // 创建三张图片 分别是像素图片 法线图片 和深度图片
    Image image(w, h);
    Image nimage(w, h);
    Image dimage(w, h);

    // loop through all the pixels in the image 循环遍历图片上的每个像素
    // generate all the samples 生成所有的采样

    RenderUtils::initRandomSeed(); // 初始化随机数种子 防止每次启动程序生成的随机数都一样

    // This look generates camera rays and callse traceRay.
    // It also write to the color, normal, and depth images.
    // You should understand what this code does.

    // -- 对于cam-generateRay的注释
    // Use PerspectiveCamera to generate a ray.
    // You should understand what generateRay() does.
    Camera *cam = _scene.getCamera();
    for (int y = 0; y < h; ++y) { // 可以假设hit=200 那么就是0-199
        float ndcy = 2 * (y / (h - 1.0f)) - 1.0f; // y/(hit-1.0f)会映射到[0,1] 那么ndcy会映射到[-1,1]
        for (int x = 0; x < w; ++x) {
            float ndcx = 2 * (x / (w - 1.0f)) - 1.0f; // ndcx同样映射到[-1,1] 于是整个循环是从下到上 从左到右逐像素绘制
            Vector2f screenOffsets = Vector2f(ndcx, ndcy);
            Vector3f color, normal;
            Ray ray; Hit hit;
            float t = 0.0f; 
            
            // 超采样抗锯齿处理
            if (_args.jitter && _args.filter && _args.samples > 1)
            {
                Vector2f sampleOffsets;
                for (int i = 0; i < _args.samples; i++)
                {
                    // Hit要声明新的 不能复用 因为物体Intersect方法 会判断现有hit里面的t来更新t 所以会出错
                    hit.reset();  // 或者新创建一个reset方法
                    sampleOffsets = RenderUtils::sample2D(screenOffsets, w, h);
                    ray = cam->generateRay(sampleOffsets);
                    color += traceRay(ray, cam->getTMin(), _args.bounces, hit);
                    normal += hit.getNormal(); // 法线和深度图像 也进行抗锯齿处理
                    t += hit.getT();
                }
                color /= float(_args.samples); // 平均采样值
                normal /= float(_args.samples);
                t /= float(_args.samples);
            }
            else // 只计算一条光线
            {
                ray = cam->generateRay(screenOffsets);
                color = traceRay(ray, cam->getTMin(), _args.bounces, hit);
                normal = hit.getNormal();
                t = hit.getT();
            }

            image.setPixel(x, y, color); // 这个x和y不能超出w和hit，否则会有段错误
            nimage.setPixel(x, y, (normal + 1.0f) / 2.0f);
            float range = (_args.depth_max - _args.depth_min);
            if (range) {
                dimage.setPixel(x, y, Vector3f((t - _args.depth_min) / range));
            }
        }
    }
    // END SOLN

    // save the files
    if (_args.output_file.size()) {
        image.savePNG(_args.output_file);
    }
    if (_args.depth_file.size()) {
        dimage.savePNG(_args.depth_file); // 命令行参数包含'depth'才会生成
    }
    if (_args.normals_file.size()) { 
        nimage.savePNG(_args.normals_file); // 命令行包含'normals'才会生成
    }
}

Vector3f
Renderer::traceRay(const Ray &r,
                   float tmin,
                   int bounces,
                   Hit &h) const {
    // The starter code only implements basic drawing of sphere primitives.
    // You will implement phong shading, recursive ray tracing, and shadow rays.

    // TODO: IMPLEMENT

    //if (_scene.getGroup()->intersect(r, tmin, h)) {
    //    Vector3f I = _scene.getAmbientLight() * h.getMaterial()->getDiffuseColor();
    //    Vector3f p = r.pointAtParameter(h.getT());
    //    for (int i = 0; i < _scene.getNumLights(); ++i) {
    //        Vector3f tolight;
    //        Vector3f intensity;
    //        float distToLight;
    //        _scene.getLight(i)->getIllumination(p, tolight, intensity, distToLight);
    //        Vector3f ILight = h.getMaterial()->shade(r, h, tolight, intensity);
    //        // To compute cast shadows, you will send rays from the surface point to each
    //        // light source. If an intersection is reported, and the intersection is closer
    //        // than the distance to the light source, the current surface point is in shadow
    //        // and direct illumination from that light source is ignored. Note that shadow
    //        // rays must be sent to all light sources.
    //        if (_args.shadows) {
    //            Vector3f shadowRayOrigin = p + 0.05f * tolight;
    //            Ray shadowRay(shadowRayOrigin, tolight);
    //            Hit shadowHit = Hit();
    //            Vector3f shadowTrace = traceRay(shadowRay, 0, 0, shadowHit);
    //            bool shadowIntersectedSomething = shadowHit.getT() < std::numeric_limits<float>::max();
    //            float distToIntersection = (shadowRay.pointAtParameter(shadowHit.getT()) - shadowRayOrigin).abs();
    //            if (
    //                shadowIntersectedSomething && distToIntersection < distToLight
    //            ) {
    //                ILight = Vector3f(0); // Object in shadow from this light, discount light.
    //            }
    //        }
    //        I += ILight;
    //    }
    //    // Reflections.
    //    if (bounces > 0) {
    //        // Recursive call.
    //        Vector3f V = r.getDirection();
    //        Vector3f N = h.getNormal().normalized();
    //        Vector3f R = (V - (2 * Vector3f::dot(V, N) * N)).normalized();
    //        Hit hPrime = Hit();
    //        // Add a little epsilon to avoid noise.
    //        Ray rPrime(p + 0.01f * R, R);
    //        Vector3f IIndirect = traceRay(rPrime, 0.0f, bounces - 1, hPrime);
    //        I += (h.getMaterial()->getSpecularColor() * IIndirect);
    //    }
    //    return I;
    //} else {
    //    return _scene.getBackgroundColor(r.getDirection());
    //};

    // To compute cast shadows, you will send rays from the surface point to each
    // light source. If an intersection is reported, and the intersection is closer
    // than the distance to the light source, the current surface point is in shadow
    // and direct illumination from that light source is ignored. Note that shadow
    // rays must be sent to all light sources.

    // 遍历场景中的所有物体 计算该条光线与场景物体最近的交点 
    // 1. 如果不存在交点 直接返回一个背景色
    // 2. 判断这个交点是否存在阴影中 如果在阴影中 仍然返回背景色
    // 3. 如果光线和场景物体存在交点 那么该点的颜色 由该交点材质的着色方法来计算
    // 4. 如果交点材质是反射或者投射型 则进行二次光线计算 即光线追踪

    Vector3f color(0.0f); // 要计算的像素值 初始化是0（黑色）
    if (_scene.getGroup()->intersect(r, tmin, h))
    {
        // 一、直接光照
        // 1. 环境光 环境光只计算一次 是包含所有光源的一个假设 无论是否处于阴影中
        color += _scene.getAmbientLight() * h.getMaterial()->getDiffuseColor();

        // 2. 漫射/反射 阴影
        // 存在视线和物体的交点 则判断该交点与所有光源的连线是否有遮挡 若有 则表示该条光线会产生阴影
        // 叠加所有不产生阴影的光线着色的像素值
        Vector3f intersectPoint = r.pointAtParameter(h.getT());

        for (Light* light : _scene.lights)
        {
            Vector3f toLight, intensity;
            float distToLight;
            // 获取交点朝向光源的方向向量 交点的光照强度 以及交点与光源的距离
            light->getIllumination(intersectPoint, toLight, intensity, distToLight);
            // 计算阴影
            if (_args.shadows)
            {
                Ray shadowRay(intersectPoint, toLight); // 发射阴影(物体->光源)光线
                Hit shadowRayHit;
                float tmin = 0.01f; // 这里似乎不能设置为0 为啥？
                if (_scene.getGroup()->intersectShadowRay(shadowRay, tmin, shadowRayHit))
                {
                    // 在物体交点与光源的连线上有其他的物体遮挡 即产生阴影 就不计算着色了
                    continue;
                }
            }
            color += h.getMaterial()->shade(r, h, toLight, intensity); // phong氏着色 包括漫射和反射
        }

        // 二、间接光照 反射&折射
        if (!h.getMaterial()->isSpecular() && !h.getMaterial()->isTransparent()) // 即不是金属材质 也不是透明材质 不需要光线追踪了
            return color;

        if (bounces <= 0 )  // 递归条件
            return color; 

        // 1. 没有折射(只有反射）
        if (!h.getMaterial()->isTransparent())
        {
            color += reflectRay(h.getMaterial()->getSpecularColor(), r.getDirection(), h.getNormal(), intersectPoint, bounces);
        }
        // 2. 有折射(反射折射都有，或者只有折射）
        else 
        {
            // 既然有折射 就需要考虑到两种介质 是进入介质还是从介质离开
            const Vector3f& rayDir = r.getDirection();
            Vector3f normal = h.getNormal(); // 这里可能必须要构造一个vector了 因为normal可能会倒置 后面的计算都是基于这个倒置的算的
            float materialRefractedIndex = h.getMaterial()->getRefractedIndex();
            float relativeRefractiveIndex = 1.0;

            if (Vector3f::dot(rayDir, normal) > 0) // 判断光线从空气中射入介质 还是从介质中出去到空气中 
            {
                normal = -normal; // 如果光线从介质中离开进入空气 设置内表面法线为外法线的倒置
                relativeRefractiveIndex = materialRefractedIndex; // materialRefractedIndex / 1.0
            }
            else
            {
                relativeRefractiveIndex = 1 / materialRefractedIndex;
            }

            const Vector3f& specularColor = h.getMaterial()->getSpecularColor();
            const Vector3f& transparentColor = h.getMaterial()->getTransparentColor();
            // 2.1 既有反射 又有折射
            if (h.getMaterial()->isSpecular())
            {
                // 一周实现光追的trick 一次只产生一条散射光线 要么反射 要么折射 根据反射率来决定
                float cosTheta = Vector3f::dot(-rayDir, normal); // 注意方向 此处取正值
                float reflectedRate = RenderUtils::calReflectedRate(cosTheta, materialRefractedIndex);  // 反射率 跟角度、折射率有关
                // 如果反射率大 表示反射几率大 取反射光线 反之取折射光线
                if (RenderUtils::randomFloat() < reflectedRate)  // 取反射光线
                {
                    color += reflectRay(specularColor, rayDir, normal, intersectPoint, bounces);
                }
                else // 取折射光线 折射光线主要考虑到 全反射的情况 又要回到反射光线
                {
                    color += refractRay(specularColor, transparentColor, rayDir, normal, intersectPoint, relativeRefractiveIndex, bounces);
                }
            }
            else // 只有折射        // 采用如此if else分支的目的 是防止上面写两遍rayDir normal
            {
                color += refractRay(specularColor, transparentColor, rayDir, normal, intersectPoint, relativeRefractiveIndex, bounces);
            }
        }
        
    }
    else // 光线与场景物体没有任何相交 返回背景色
    {
        color = _scene.getBackgroundColor(r.getDirection());
    }
    return color;
}


Vector3f Renderer::reflectRay(const Vector3f& specularColor, const Vector3f& rayDir, const Vector3f& normal,
                              const Vector3f& intersectPoint, int bounces) const
{
    Vector3f reflectedRayDir = rayDir - 2 * (Vector3f::dot(rayDir, normal)) * normal;
    Ray reflectedRay(intersectPoint, reflectedRayDir);
    Hit reflectedHit;
    return specularColor * traceRay(reflectedRay, 0.01f, bounces - 1, reflectedHit); // 反射了一次 下一个递归就减少一次bounds
}

// 折射光线考虑到可能会发生全反射 所以多传了一个specularColor下来
Vector3f Renderer::refractRay(const Vector3f& specularColor, const Vector3f& transparentColor, const Vector3f& rayDir, const Vector3f& normal,
                              const Vector3f& intersectPoint, float relativeRefractiveIndex, int bounces) const
{
    // 传递参数 计算折射光线(的方向)
    Vector3f refractedRayDir;
    if (RenderUtils::hasRefractedRay(rayDir, normal, relativeRefractiveIndex, refractedRayDir)) // 有折射光线
    {
        Ray refratedRay(intersectPoint, refractedRayDir);
        Hit refracedHit;
        return transparentColor * traceRay(refratedRay, 0.01f, bounces - 1, refracedHit);
    }
    else // 全反射 没有折射光线 仍然只能走反射
    {
        return reflectRay(specularColor, rayDir, normal, intersectPoint, bounces);
    }
}


