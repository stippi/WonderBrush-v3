#ifndef STACK_BLUR_FILTER
#define STACK_BLUR_FILTER

#include <SupportDefs.h>

class BBitmap;
class RenderBuffer;

class StackBlurFilter {
 public:
								StackBlurFilter();
								~StackBlurFilter();


			void				FilterRGBA64(RenderBuffer* buffer, double radius);
			void				FilterRGBA32(RenderBuffer* buffer, double radius);
			void				FilterGray8(RenderBuffer* buffer, double radius);
			void				Filter(BBitmap* bitmap, double radius);

 private:
			void				_Filter64(uint16* buffer,
										  unsigned width, unsigned height,
										  int32 bpr,
										  unsigned rx, unsigned ry) const;
			void				_Filter32(uint8* buffer,
										  unsigned width, unsigned height,
										  int32 bpr,
										  unsigned rx, unsigned ry) const;
			void				_Filter8(uint8* buffer,
										 unsigned width, unsigned height,
										 int32 bpr,
										 unsigned rx, unsigned ry) const;
};

#endif // STACK_BLUR_FILTER
