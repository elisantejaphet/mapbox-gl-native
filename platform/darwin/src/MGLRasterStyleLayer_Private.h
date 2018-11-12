// This file is generated.
// Edit platform/darwin/scripts/generate-style-code.js, then run `make darwin-style-code`.

#import "MGLStyleLayerManager.h"

NS_ASSUME_NONNULL_BEGIN

@interface MGLRasterStyleLayerFactory : NSObject <MGLStyleLayerFactory>

- (MGLStyleLayer *)createWithRawLayer:(mbgl::style::Layer *)rawLayer;

@property (nonatomic, readonly) mbgl::style::LayerFactory *rawLayerFactory;

@end

NS_ASSUME_NONNULL_END
