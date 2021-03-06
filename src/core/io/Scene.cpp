#include "Scene.hpp"
#include "DirectoryChange.hpp"
#include "JsonUtils.hpp"
#include "FileUtils.hpp"

#include "integrators/PathTraceIntegrator.hpp"

#include "primitives/InfiniteSphereCap.hpp"
#include "primitives/InfiniteSphere.hpp"
#include "primitives/TriangleMesh.hpp"
#include "primitives/Sphere.hpp"
#include "primitives/Curves.hpp"
#include "primitives/Quad.hpp"
#include "primitives/Disk.hpp"

#include "materials/ConstantTexture.hpp"
#include "materials/CheckerTexture.hpp"
#include "materials/BladeTexture.hpp"
#include "materials/DiskTexture.hpp"

#include "cameras/ThinlensCamera.hpp"
#include "cameras/PinholeCamera.hpp"

#include "volume/HomogeneousMedium.hpp"
#include "volume/AtmosphericMedium.hpp"

#include "bsdfs/RoughDielectricBsdf.hpp"
#include "bsdfs/RoughConductorBsdf.hpp"
#include "bsdfs/RoughPlasticBsdf.hpp"
#include "bsdfs/TransparencyBsdf.hpp"
#include "bsdfs/DielectricBsdf.hpp"
#include "bsdfs/SmoothCoatBsdf.hpp"
#include "bsdfs/RoughCoatBsdf.hpp"
#include "bsdfs/ConductorBsdf.hpp"
#include "bsdfs/OrenNayarBsdf.hpp"
#include "bsdfs/ThinSheetBsdf.hpp"
#include "bsdfs/ForwardBsdf.hpp"
#include "bsdfs/LambertBsdf.hpp"
#include "bsdfs/PlasticBsdf.hpp"
#include "bsdfs/MirrorBsdf.hpp"
#include "bsdfs/PhongBsdf.hpp"
#include "bsdfs/MixedBsdf.hpp"
#include "bsdfs/NullBsdf.hpp"
#include "bsdfs/Bsdf.hpp"

#include "Debug.hpp"

#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <functional>

namespace Tungsten {

Scene::Scene()
: _camera(std::make_shared<PinholeCamera>()),
  _integrator(std::make_shared<PathTraceIntegrator>())
{
}

Scene::Scene(const std::string &srcDir, std::shared_ptr<TextureCache> cache)
: _srcDir(srcDir),
  _textureCache(std::move(cache)),
  _camera(std::make_shared<PinholeCamera>()),
  _integrator(std::make_shared<PathTraceIntegrator>())
{
}

Scene::Scene(const std::string &srcDir,
      std::vector<std::shared_ptr<Primitive>> primitives,
      std::vector<std::shared_ptr<Bsdf>> bsdfs,
      std::shared_ptr<TextureCache> cache,
      std::shared_ptr<Camera> camera)
: _srcDir(srcDir),
  _primitives(std::move(primitives)),
  _bsdfs(std::move(bsdfs)),
  _textureCache(std::move(cache)),
  _camera(std::move(camera)),
  _integrator(std::make_shared<PathTraceIntegrator>())
{
}

std::shared_ptr<Medium> Scene::instantiateMedium(std::string type, const rapidjson::Value &value) const
{
    std::shared_ptr<Medium> result;
    if (type == "homogeneous")
        result = std::make_shared<HomogeneousMedium>();
    else if (type == "atmosphere")
        result = std::make_shared<AtmosphericMedium>();
    else
        FAIL("Unkown medium type: '%s'", type.c_str());
    result->fromJson(value, *this);
    return result;
}

std::shared_ptr<Bsdf> Scene::instantiateBsdf(std::string type, const rapidjson::Value &value) const
{
    std::shared_ptr<Bsdf> result;
    if (type == "lambert")
        result = std::make_shared<LambertBsdf>();
    else if (type == "phong")
        result = std::make_shared<PhongBsdf>();
    else if (type == "mixed")
        result = std::make_shared<MixedBsdf>();
    else if (type == "dielectric")
        result = std::make_shared<DielectricBsdf>();
    else if (type == "conductor")
        result = std::make_shared<ConductorBsdf>();
    else if (type == "mirror")
        result = std::make_shared<MirrorBsdf>();
    else if (type == "rough_conductor")
        result = std::make_shared<RoughConductorBsdf>();
    else if (type == "rough_dielectric")
        result = std::make_shared<RoughDielectricBsdf>();
    else if (type == "smooth_coat")
        result = std::make_shared<SmoothCoatBsdf>();
    else if (type == "null")
        result = std::make_shared<NullBsdf>();
    else if (type == "forward")
        result = std::make_shared<ForwardBsdf>();
    else if (type == "thinsheet")
        result = std::make_shared<ThinSheetBsdf>();
    else if (type == "oren_nayar")
        result = std::make_shared<OrenNayarBsdf>();
    else if (type == "plastic")
        result = std::make_shared<PlasticBsdf>();
    else if (type == "rough_plastic")
        result = std::make_shared<RoughPlasticBsdf>();
    else if (type == "rough_coat")
        result = std::make_shared<RoughCoatBsdf>();
    else if (type == "transparency")
        result = std::make_shared<TransparencyBsdf>();
    else
        FAIL("Unkown bsdf type: '%s'", type.c_str());
    result->fromJson(value, *this);
    return result;
}

std::shared_ptr<Primitive> Scene::instantiatePrimitive(std::string type, const rapidjson::Value &value) const
{
    std::shared_ptr<Primitive> result;
    if (type == "mesh")
        result = std::make_shared<TriangleMesh>();
    else if (type == "sphere")
        result = std::make_shared<Sphere>();
    else if (type == "quad")
        result = std::make_shared<Quad>();
    else if (type == "disk")
        result = std::make_shared<Disk>();
    else if (type == "infinite_sphere")
        result = std::make_shared<InfiniteSphere>();
    else if (type == "infinite_sphere_cap")
        result = std::make_shared<InfiniteSphereCap>();
    else if (type == "curves")
        result = std::make_shared<Curves>();
    else
        FAIL("Unknown primitive type: '%s'", type.c_str());

    result->fromJson(value, *this);
    return result;
}

std::shared_ptr<Camera> Scene::instantiateCamera(std::string type, const rapidjson::Value &value) const
{
    std::shared_ptr<Camera> result;
    if (type == "pinhole")
        result = std::make_shared<PinholeCamera>();
    else if (type == "thinlens")
        result = std::make_shared<ThinlensCamera>();
    else
        FAIL("Unknown camera type: '%s'", type.c_str());

    result->fromJson(value, *this);
    return result;
}

std::shared_ptr<Integrator> Scene::instantiateIntegrator(std::string type, const rapidjson::Value &value) const
{
    std::shared_ptr<Integrator> result;
    if (type == "path_trace")
        result = std::make_shared<PathTraceIntegrator>();
    else
        FAIL("Unknown integrator type: '%s'", type.c_str());

    result->fromJson(value, *this);
    return result;
}

std::shared_ptr<Texture> Scene::instantiateTexture(std::string type, const rapidjson::Value &value, TexelConversion conversion) const
{
    std::shared_ptr<Texture> result;
    if (type == "bitmap")
        return _textureCache->fetchTexture(JsonUtils::as<std::string>(value, "path"), conversion);
    else if (type == "constant")
        result = std::make_shared<ConstantTexture>();
    else if (type == "checker")
        result = std::make_shared<CheckerTexture>();
    else if (type == "disk")
        result = std::make_shared<DiskTexture>();
    else if (type == "blade")
        result = std::make_shared<BladeTexture>();
    else
        FAIL("Unkown texture type: '%s'", type.c_str());

    result->fromJson(value, *this);
    return result;
}

template<typename Instantiator, typename Element>
void Scene::loadObjectList(const rapidjson::Value &container, Instantiator instantiator, std::vector<std::shared_ptr<Element>> &result)
{
    for (unsigned i = 0; i < container.Size(); ++i) {
        if (container[i].IsObject())
            result.push_back(instantiator(JsonUtils::as<std::string>(container[i], "type"), container[i]));
        else
            DBG("Don't know what to do with non-object in object list");
    }
}

template<typename T>
std::shared_ptr<T> Scene::findObject(const std::vector<std::shared_ptr<T>> &list, std::string name) const
{
    for (const std::shared_ptr<T> &t : list)
        if (t->name() == name)
            return t;
    FAIL("Unable to find object '%s'", name.c_str());
    return nullptr;
}

template<typename T, typename Instantiator>
std::shared_ptr<T> Scene::fetchObject(const std::vector<std::shared_ptr<T>> &list, const rapidjson::Value &v, Instantiator instantiator) const
{
    if (v.IsString()) {
        return findObject(list, v.GetString());
    } else if (v.IsObject()) {
        return instantiator(JsonUtils::as<std::string>(v, "type"), v);
    } else {
        FAIL("Unkown value type");
        return nullptr;
    }
}

std::shared_ptr<Medium> Scene::fetchMedium(const rapidjson::Value &v) const
{
    using namespace std::placeholders;
    return fetchObject(_media, v, std::bind(&Scene::instantiateMedium, this, _1, _2));
}

std::shared_ptr<Bsdf> Scene::fetchBsdf(const rapidjson::Value &v) const
{
    using namespace std::placeholders;
    return fetchObject(_bsdfs, v, std::bind(&Scene::instantiateBsdf, this, _1, _2));
}

std::shared_ptr<Texture> Scene::fetchTexture(const rapidjson::Value &v, TexelConversion conversion) const
{
    // Note: TexelConversions are only honored by BitmapTexture.
    // This is inconsistent, but conversions do not really make sense for other textures,
    // unless the user expects e.g. a ConstantTexture with Vec3 argument to select the green
    // channel when used in a TransparencyBsdf.
    if (v.IsString())
        return _textureCache->fetchTexture(v.GetString(), conversion);
    else if (v.IsNumber())
        return std::make_shared<ConstantTexture>(JsonUtils::as<float>(v));
    else if (v.IsArray())
        return std::make_shared<ConstantTexture>(JsonUtils::as<Vec3f>(v));
    else if (v.IsObject())
        return instantiateTexture(JsonUtils::as<std::string>(v, "type"), v, conversion);
    else
        FAIL("Cannot instantiate texture from unknown value type");
    return nullptr;
}

bool Scene::textureFromJsonMember(const rapidjson::Value &v, const char *field, TexelConversion conversion,
        std::shared_ptr<Texture> &dst) const
{
    const rapidjson::Value::Member *member = v.FindMember(field);
    if (!member)
        return false;

    std::shared_ptr<Texture> tex = fetchTexture(member->value, conversion);
    if (!tex)
        return false;

    dst = std::move(tex);
    return true;
}

const Primitive *Scene::findPrimitive(const std::string &name) const
{
    for (const std::shared_ptr<Primitive> &m : _primitives)
        if (m->name() == name)
            return m.get();
    return nullptr;
}

template<typename T>
bool Scene::addUnique(const std::shared_ptr<T> &o, std::vector<std::shared_ptr<T>> &list)
{
    bool retry = false;
    int dupeCount = 0;
    std::string baseName = o->name();
    if (baseName.empty())
        return true;
    for (int i = 1; !baseName.empty() && isdigit(baseName.back()); i *= 10, baseName.pop_back())
        dupeCount += i*(baseName.back() - '0');
    std::string newName = o->name();
    do {
        retry = false;
        for (const std::shared_ptr<T> &m : list) {
            if (m.get() == o.get())
                return false;
            if (m->name() == newName) {
                newName = tfm::format("%s%d", baseName, ++dupeCount);
                retry = true;
                break;
            }
        }
    } while (retry);

    o->setName(newName);
    list.push_back(o);

    return true;
}

void Scene::addPrimitive(const std::shared_ptr<Primitive> &mesh)
{
    if (addUnique(mesh, _primitives))
        addBsdf(mesh->bsdf());
}

void Scene::addBsdf(const std::shared_ptr<Bsdf> &bsdf)
{
    if (addUnique(bsdf, _bsdfs)) {
        if (bsdf->intMedium())
            addUnique(bsdf->intMedium(), _media);
        if (bsdf->extMedium())
            addUnique(bsdf->extMedium(), _media);
    }
}

void Scene::merge(Scene scene)
{
    for (std::shared_ptr<Primitive> &m : scene._primitives)
        addPrimitive(m);
}

void Scene::fromJson(const rapidjson::Value &v, const Scene &scene)
{
    JsonSerializable::fromJson(v, scene);

    using namespace std::placeholders;

    const rapidjson::Value::Member *media      = v.FindMember("media");
    const rapidjson::Value::Member *bsdfs      = v.FindMember("bsdfs");
    const rapidjson::Value::Member *primitives = v.FindMember("primitives");
    const rapidjson::Value::Member *camera     = v.FindMember("camera");
    const rapidjson::Value::Member *integrator = v.FindMember("integrator");
    const rapidjson::Value::Member *renderer   = v.FindMember("renderer");

    if (media && media->value.IsArray())
        loadObjectList(media->value, std::bind(&Scene::instantiateMedium, this, _1, _2), _media);

    if (bsdfs && bsdfs->value.IsArray())
        loadObjectList(bsdfs->value, std::bind(&Scene::instantiateBsdf, this, _1, _2), _bsdfs);

    if (primitives && primitives->value.IsArray())
        loadObjectList(primitives->value, std::bind(&Scene::instantiatePrimitive, this, _1, _2), _primitives);

    if (camera && camera->value.IsObject())
        _camera = instantiateCamera(JsonUtils::as<std::string>(camera->value, "type"), camera->value);

    if (integrator && integrator->value.IsObject())
        _integrator = instantiateIntegrator(JsonUtils::as<std::string>(integrator->value, "type"), integrator->value);

    if (renderer && renderer->value.IsObject())
        _rendererSettings.fromJson(renderer->value, *this);
}

rapidjson::Value Scene::toJson(Allocator &allocator) const
{
    rapidjson::Value v = JsonSerializable::toJson(allocator);

    rapidjson::Value media(rapidjson::kArrayType);
    for (const std::shared_ptr<Medium> &b : _media)
        media.PushBack(b->toJson(allocator), allocator);

    rapidjson::Value bsdfs(rapidjson::kArrayType);
    for (const std::shared_ptr<Bsdf> &b : _bsdfs)
        bsdfs.PushBack(b->toJson(allocator), allocator);

    rapidjson::Value primitives(rapidjson::kArrayType);
    for (const std::shared_ptr<Primitive> &t : _primitives)
        primitives.PushBack(t->toJson(allocator), allocator);

    v.AddMember("media", media, allocator);
    v.AddMember("bsdfs", bsdfs, allocator);
    v.AddMember("primitives", primitives, allocator);
    v.AddMember("camera", _camera->toJson(allocator), allocator);
    v.AddMember("integrator", _integrator->toJson(allocator), allocator);
    v.AddMember("renderer", _rendererSettings.toJson(allocator), allocator);

    return std::move(v);
}

void Scene::deletePrimitives(const std::unordered_set<Primitive *> &primitives)
{
    std::vector<std::shared_ptr<Primitive>> newPrims;
    newPrims.reserve(_primitives.size());

    for (std::shared_ptr<Primitive> &m : _primitives)
        if (!primitives.count(m.get()))
            newPrims.push_back(m);

    _primitives = std::move(newPrims);
}

TraceableScene *Scene::makeTraceable()
{
    return new TraceableScene(*_camera, *_integrator, _primitives, _media, _rendererSettings);
}

Scene *Scene::load(const std::string &path, std::shared_ptr<TextureCache> cache)
{
    std::string json;
    try {
        json = FileUtils::loadText(path.c_str());
    } catch (const std::runtime_error &) {
        throw std::runtime_error(tfm::format("Unable to open file at '%s'", path));
    }

    rapidjson::Document document;
    document.Parse<0>(json.c_str());
    if (document.HasParseError())
        throw std::runtime_error(tfm::format("JSON parse error: %s", document.GetParseError()));

    DirectoryChange context(FileUtils::extractParent(path));

    if (!cache)
        cache = std::make_shared<TextureCache>();

    Scene *scene = new Scene(FileUtils::extractParent(path), std::move(cache));
    scene->fromJson(document, *scene);
    scene->setPath(path);

    return scene;
}

void Scene::save(const std::string &path, const Scene &scene)
{
    rapidjson::Document document;
    document.SetObject();

    *(static_cast<rapidjson::Value *>(&document)) = scene.toJson(document.GetAllocator());

    std::unique_ptr<char[]> buffer(new char[4*1024]);
    FILE *fp = fopen(path.c_str(), "wb");
    rapidjson::FileWriteStream out(fp, buffer.get(), 4*1024);

    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(out);
    document.Accept(writer);
    fclose(fp);
}

}
