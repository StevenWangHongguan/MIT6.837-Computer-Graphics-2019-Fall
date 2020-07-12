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
    // ͼƬ�ĸ߿�
    int w = _args.width;
    int h = _args.height;

    // ��������ͼƬ �ֱ�������ͼƬ ����ͼƬ �����ͼƬ
    Image image(w, h);
    Image nimage(w, h);
    Image dimage(w, h);

    // loop through all the pixels in the image ѭ������ͼƬ�ϵ�ÿ������
    // generate all the samples �������еĲ���

    RenderUtils::initRandomSeed(); // ��ʼ����������� ��ֹÿ�������������ɵ��������һ��

    // This look generates camera rays and callse traceRay.
    // It also write to the color, normal, and depth images.
    // You should understand what this code does.

    // -- ����cam-generateRay��ע��
    // Use PerspectiveCamera to generate a ray.
    // You should understand what generateRay() does.
    Camera *cam = _scene.getCamera();
    for (int y = 0; y < h; ++y) { // ���Լ���hit=200 ��ô����0-199
        float ndcy = 2 * (y / (h - 1.0f)) - 1.0f; // y/(hit-1.0f)��ӳ�䵽[0,1] ��ôndcy��ӳ�䵽[-1,1]
        for (int x = 0; x < w; ++x) {
            float ndcx = 2 * (x / (w - 1.0f)) - 1.0f; // ndcxͬ��ӳ�䵽[-1,1] ��������ѭ���Ǵ��µ��� �����������ػ���
            Vector2f screenOffsets = Vector2f(ndcx, ndcy);
            Vector3f color, normal;
            Ray ray; Hit hit;
            float t = 0.0f; 
            
            // ����������ݴ���
            if (_args.jitter && _args.filter && _args.samples > 1)
            {
                Vector2f sampleOffsets;
                for (int i = 0; i < _args.samples; i++)
                {
                    // HitҪ�����µ� ���ܸ��� ��Ϊ����Intersect���� ���ж�����hit�����t������t ���Ի����
                    hit.reset();  // �����´���һ��reset����
                    sampleOffsets = RenderUtils::sample2D(screenOffsets, w, h);
                    ray = cam->generateRay(sampleOffsets);
                    color += traceRay(ray, cam->getTMin(), _args.bounces, hit);
                    normal += hit.getNormal(); // ���ߺ����ͼ�� Ҳ���п���ݴ���
                    t += hit.getT();
                }
                color /= float(_args.samples); // ƽ������ֵ
                normal /= float(_args.samples);
                t /= float(_args.samples);
            }
            else // ֻ����һ������
            {
                ray = cam->generateRay(screenOffsets);
                color = traceRay(ray, cam->getTMin(), _args.bounces, hit);
                normal = hit.getNormal();
                t = hit.getT();
            }

            image.setPixel(x, y, color); // ���x��y���ܳ���w��hit��������жδ���
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
        dimage.savePNG(_args.depth_file); // �����в�������'depth'�Ż�����
    }
    if (_args.normals_file.size()) { 
        nimage.savePNG(_args.normals_file); // �����а���'normals'�Ż�����
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

    // ���������е��������� ������������볡����������Ľ��� 
    // 1. ��������ڽ��� ֱ�ӷ���һ������ɫ
    // 2. �ж���������Ƿ������Ӱ�� �������Ӱ�� ��Ȼ���ر���ɫ
    // 3. ������ߺͳ���������ڽ��� ��ô�õ����ɫ �ɸý�����ʵ���ɫ����������
    // 4. �����������Ƿ������Ͷ���� ����ж��ι��߼��� ������׷��

    Vector3f color(0.0f); // Ҫ���������ֵ ��ʼ����0����ɫ��
    if (_scene.getGroup()->intersect(r, tmin, h))
    {
        // һ��ֱ�ӹ���
        // 1. ������ ������ֻ����һ�� �ǰ������й�Դ��һ������ �����Ƿ�����Ӱ��
        color += _scene.getAmbientLight() * h.getMaterial()->getDiffuseColor();

        // 2. ����/���� ��Ӱ
        // �������ߺ�����Ľ��� ���жϸý��������й�Դ�������Ƿ����ڵ� ���� ���ʾ�������߻������Ӱ
        // �������в�������Ӱ�Ĺ�����ɫ������ֵ
        Vector3f intersectPoint = r.pointAtParameter(h.getT());

        for (Light* light : _scene.lights)
        {
            Vector3f toLight, intensity;
            float distToLight;
            // ��ȡ���㳯���Դ�ķ������� ����Ĺ���ǿ�� �Լ��������Դ�ľ���
            light->getIllumination(intersectPoint, toLight, intensity, distToLight);
            // ������Ӱ
            if (_args.shadows)
            {
                Ray shadowRay(intersectPoint, toLight); // ������Ӱ(����->��Դ)����
                Hit shadowRayHit;
                float tmin = 0.01f; // �����ƺ���������Ϊ0 Ϊɶ��
                if (_scene.getGroup()->intersectShadowRay(shadowRay, tmin, shadowRayHit))
                {
                    // �����彻�����Դ���������������������ڵ� ��������Ӱ �Ͳ�������ɫ��
                    continue;
                }
            }
            color += h.getMaterial()->shade(r, h, toLight, intensity); // phong����ɫ ��������ͷ���
        }

        // ������ӹ��� ����&����
        if (!h.getMaterial()->isSpecular() && !h.getMaterial()->isTransparent()) // �����ǽ������� Ҳ����͸������ ����Ҫ����׷����
            return color;

        if (bounces <= 0 )  // �ݹ�����
            return color; 

        // 1. û������(ֻ�з��䣩
        if (!h.getMaterial()->isTransparent())
        {
            color += reflectRay(h.getMaterial()->getSpecularColor(), r.getDirection(), h.getNormal(), intersectPoint, bounces);
        }
        // 2. ������(�������䶼�У�����ֻ�����䣩
        else 
        {
            // ��Ȼ������ ����Ҫ���ǵ����ֽ��� �ǽ�����ʻ��Ǵӽ����뿪
            const Vector3f& rayDir = r.getDirection();
            Vector3f normal = h.getNormal(); // ������ܱ���Ҫ����һ��vector�� ��Ϊnormal���ܻᵹ�� ����ļ��㶼�ǻ���������õ����
            float materialRefractedIndex = h.getMaterial()->getRefractedIndex();
            float relativeRefractiveIndex = 1.0;

            if (Vector3f::dot(rayDir, normal) > 0) // �жϹ��ߴӿ������������ ���Ǵӽ����г�ȥ�������� 
            {
                normal = -normal; // ������ߴӽ������뿪������� �����ڱ��淨��Ϊ�ⷨ�ߵĵ���
                relativeRefractiveIndex = materialRefractedIndex; // materialRefractedIndex / 1.0
            }
            else
            {
                relativeRefractiveIndex = 1 / materialRefractedIndex;
            }

            const Vector3f& specularColor = h.getMaterial()->getSpecularColor();
            const Vector3f& transparentColor = h.getMaterial()->getTransparentColor();
            // 2.1 ���з��� ��������
            if (h.getMaterial()->isSpecular())
            {
                // һ��ʵ�ֹ�׷��trick һ��ֻ����һ��ɢ����� Ҫô���� Ҫô���� ���ݷ�����������
                float cosTheta = Vector3f::dot(-rayDir, normal); // ע�ⷽ�� �˴�ȡ��ֵ
                float reflectedRate = RenderUtils::calReflectedRate(cosTheta, materialRefractedIndex);  // ������ ���Ƕȡ��������й�
                // ��������ʴ� ��ʾ���伸�ʴ� ȡ������� ��֮ȡ�������
                if (RenderUtils::randomFloat() < reflectedRate)  // ȡ�������
                {
                    color += reflectRay(specularColor, rayDir, normal, intersectPoint, bounces);
                }
                else // ȡ������� ���������Ҫ���ǵ� ȫ�������� ��Ҫ�ص��������
                {
                    color += refractRay(specularColor, transparentColor, rayDir, normal, intersectPoint, relativeRefractiveIndex, bounces);
                }
            }
            else // ֻ������        // �������if else��֧��Ŀ�� �Ƿ�ֹ����д����rayDir normal
            {
                color += refractRay(specularColor, transparentColor, rayDir, normal, intersectPoint, relativeRefractiveIndex, bounces);
            }
        }
        
    }
    else // �����볡������û���κ��ཻ ���ر���ɫ
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
    return specularColor * traceRay(reflectedRay, 0.01f, bounces - 1, reflectedHit); // ������һ�� ��һ���ݹ�ͼ���һ��bounds
}

// ������߿��ǵ����ܻᷢ��ȫ���� ���Զഫ��һ��specularColor����
Vector3f Renderer::refractRay(const Vector3f& specularColor, const Vector3f& transparentColor, const Vector3f& rayDir, const Vector3f& normal,
                              const Vector3f& intersectPoint, float relativeRefractiveIndex, int bounces) const
{
    // ���ݲ��� �����������(�ķ���)
    Vector3f refractedRayDir;
    if (RenderUtils::hasRefractedRay(rayDir, normal, relativeRefractiveIndex, refractedRayDir)) // ���������
    {
        Ray refratedRay(intersectPoint, refractedRayDir);
        Hit refracedHit;
        return transparentColor * traceRay(refratedRay, 0.01f, bounces - 1, refracedHit);
    }
    else // ȫ���� û��������� ��Ȼֻ���߷���
    {
        return reflectRay(specularColor, rayDir, normal, intersectPoint, bounces);
    }
}


