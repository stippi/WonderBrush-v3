#ifndef STACK_BLUR_FILTER
#define STACK_BLUR_FILTER

#include <SupportDefs.h>

class AlphaBuffer;
class BBitmap;
class RenderBuffer;

class StackBlurFilter {
public:
								StackBlurFilter();
								~StackBlurFilter();


			void				FilterRGBA64(RenderBuffer* buffer, double radius);
			void				FilterRGBA32(RenderBuffer* buffer, double radius);
			void				FilterGray16(AlphaBuffer* buffer, double radius);
			void				Filter(BBitmap* bitmap, double radius);

private:
			void				_Filter64(uint16* buffer,
									unsigned width, unsigned height,
									int32 bpr, unsigned rx, unsigned ry) const;
			void				_Filter32(uint8* buffer,
									unsigned width, unsigned height, int32 bpr,
									unsigned rx, unsigned ry) const;

			template<typename PixelType>
			void				_FilterGray(PixelType* buffer,
									unsigned width, unsigned height,
									int32 bpr, unsigned rx, unsigned ry) const;
};

#endif // STACK_BLUR_FILTER
