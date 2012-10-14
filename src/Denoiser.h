#ifndef DENOISER_H
#define DENOISER_H

#include <Application.h>
#include <Path.h>
#include <String.h>

class Denoiser : public BApplication {
public:
								Denoiser();

	virtual	void				MessageReceived(BMessage* message);
	virtual void				ReadyToRun();
	virtual	void				ArgvReceived(int32 argc, char** argv);
	virtual	void				RefsReceived(BMessage* message);

private:
			void				_PrintUsage(const char* appPath);
			status_t			_DenoiseImage(BBitmap* bitmap,
									BPath path,
									const char* originalPath) const;

private:
			bool				fPrintUsage;

			float				fAmplitude;
			float				fSharpness;
			float				fAnisotropy;
			float				fAlpha;
			float				fSigma;
			float				fDL;
			float				fDA;
			float				fGaussPrecision;
			uint32				fInterpolationType;
			bool				fFastAproximation;

			BPath				fTargetFolder;
			uint32				fTargetFormat;
};

#endif // DENOISER_H
