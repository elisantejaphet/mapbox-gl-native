#import "MGLStyleLayerManager.h"

NS_ASSUME_NONNULL_BEGIN

@interface MGLOpenGLStyleLayerFactory : NSObject <MGLStyleLayerFactory>
/**
Initializes and returns a layer with a raw pointer to the backing store,
associated with a style.
*/
- (MGLStyleLayer *)createWithRawLayer:(mbgl::style::Layer *)rawLayer;

/**
 A raw pointer to the mbgl object, which is always initialized.
 */
@property (nonatomic, readonly) mbgl::style::LayerFactory *rawLayerFactory;

@end

NS_ASSUME_NONNULL_END
