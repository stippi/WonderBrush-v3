#ifndef GAUSS_FILTER
#define GAUSS_FILTER

#include <SupportDefs.h>

class RenderBuffer;

class GaussFilter {
 public:
								GaussFilter();
								~GaussFilter();

			void				FilterRGB32(RenderBuffer* buffer, double radius);
			void				FilterRGBA32(RenderBuffer* buffer, double radius);
			void				FilterRGBA64(RenderBuffer* buffer, double radius);
			void				FilterGray8(RenderBuffer* buffer, double radius);

 private:
			typedef float CalcType;

			bool				_Init(double radius);

 			void				_Filter(CalcType* buffer, int32 count);

			template<typename ChannelType>
			void				_FilterRow4(ChannelType* buffer, int32 count,
											bool alpha);
			template<typename ChannelType>
			void				_FilterColumn4(ChannelType* buffer,
									uint32 skip, int32 count,
									bool alpha);

			template<typename ChannelType>
			void				_FilterRow1(ChannelType* buffer, int32 count);
			template<typename ChannelType>
			void				_FilterColumn1(ChannelType* buffer,
									uint32 skip, int32 count);

			CalcType			b0;
			CalcType			b1;
			CalcType			b2;
			CalcType			b3;
			CalcType			B;
};

#endif // GAUSS_FILTER
