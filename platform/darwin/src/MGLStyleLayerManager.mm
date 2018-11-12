#import "MGLStyleLayerManager.h"

#import "MGLBackgroundStyleLayer_Private.h"
#import "MGLCircleStyleLayer_Private.h"
#import "MGLFillExtrusionStyleLayer_Private.h"
#import "MGLFillStyleLayer_Private.h"
#import "MGLHeatmapStyleLayer_Private.h"
#import "MGLHillshadeStyleLayer_Private.h"
#import "MGLLineStyleLayer_Private.h"
#import "MGLRasterStyleLayer_Private.h"
#import "MGLSymbolStyleLayer_Private.h"
#import "MGLOpenGLStyleLayer_Private.h"

#include <mbgl/style/layer.hpp>
#include <vector>

namespace mbgl {
namespace style {
 
class LayerManagerMac : public LayerManager {
public:
    void addFactory(LayerFactory* factory) {
        factories.push_back(factory);
    }

private:
    // LayerManager overrides.
    std::unique_ptr<Layer> createLayer(const std::string& type, const std::string& id, const conversion::Convertible& value, conversion::Error& error) noexcept final;

    std::vector<LayerFactory*> factories;
};

std::unique_ptr<Layer> LayerManagerMac::createLayer(const std::string& type,
                                                     const std::string& id,
                                                     const conversion::Convertible& value,
                                                     conversion::Error& error) noexcept {
    for (LayerFactory* factory: factories) {
        if (factory->supportsType(type)) {
            auto layer = factory->createLayer(id, value);
            if (!layer) {
                error.message = "Error parsing a layer of type: " + type;
            }
            return layer;
        }
    }
    error.message = "Unsupported layer type: " + type;
    return nullptr;
}

} // namespace style
} // namespace mbgl

@interface MGLStyleLayerManager()

@property (nonatomic, readonly) mbgl::style::LayerManager *rawLayerManager;

@end

@implementation MGLStyleLayerManager {
    mbgl::style::LayerManagerMac _rawLayerManager;
    NSArray<id<MGLStyleLayerFactory>> *_factories;
}

+ (instancetype)get
{
    static MGLStyleLayerManager *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (instancetype)init
{
  if (self = [super init]) {
      _factories = [NSArray arrayWithObjects:
                    [[MGLFillStyleLayerFactory alloc] init],
                    [[MGLFillExtrusionStyleLayerFactory alloc] init],
                    [[MGLLineStyleLayerFactory alloc] init],
                    [[MGLSymbolStyleLayerFactory alloc] init],
                    [[MGLRasterStyleLayerFactory alloc] init],
                    [[MGLHeatmapStyleLayerFactory alloc] init],
                    [[MGLHillshadeStyleLayerFactory alloc] init],
                    [[MGLCircleStyleLayerFactory alloc] init],
                    [[MGLBackgroundStyleLayerFactory alloc] init],
                    [[MGLOpenGLStyleLayerFactory alloc] init],
                    nil];
      for (id<MGLStyleLayerFactory> factory in _factories) {
          _rawLayerManager.addFactory(factory.rawLayerFactory);
      }
  }
  return self;
}

- (mbgl::style::LayerManager *)rawLayerManager
{
    return &_rawLayerManager;
}

- (MGLStyleLayer *)createWithRawLayer:(mbgl::style::Layer *)rawLayer
{
    auto* layerFactory = rawLayer->getFactory();
    for (id<MGLStyleLayerFactory> factory in _factories) {
        if (factory.rawLayerFactory == layerFactory) {
            return [factory createWithRawLayer:rawLayer];
        }
    }
    return nil;
}

@end

namespace mbgl {
namespace style {

// static
LayerManager* LayerManager::get() noexcept {
    return [[MGLStyleLayerManager get]rawLayerManager];
}

} // namespace style
} // namespace mbgl
