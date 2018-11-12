#import <Foundation/Foundation.h>

#import "MGLStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

namespace mbgl {
    namespace style {
        class Layer;
        class LayerFactory;
    }
}

@protocol MGLStyleLayerFactory

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


@interface MGLStyleLayerManager : NSObject

/**
Initializes and returns a layer with a raw pointer to the backing store,
associated with a style.
*/
- (MGLStyleLayer *)createWithRawLayer:(mbgl::style::Layer *)rawLayer;

/**
 A singleton getter.
 */
+ (instancetype)get;

@end

NS_ASSUME_NONNULL_END
