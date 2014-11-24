#include "BasicApp.h"

// define the screen resolution
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

BasicApp::BasicApp() : _hwnd(NULL),_pDirect2dFactory(NULL), _pRenderTarget(NULL), _pLightSlateGrayBrush(NULL), _pCornflowerBlueBrush(NULL){}
BasicApp::~BasicApp(){
	SafeRelease(&_pDirect2dFactory);
	SafeRelease(&_pLightSlateGrayBrush);
	SafeRelease(&_pRenderTarget);
	SafeRelease(&_pCornflowerBlueBrush);
}

// Register the window class and call methods for instantiating drawing resources
HRESULT BasicApp::Initialize(){
	HRESULT hr;
	// Initialize device-indpendent resources, such
	// as the Direct2D factory.
	hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr)) {
		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style         = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc   = BasicApp::WndProc;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = sizeof(LONG_PTR);
		wcex.hInstance     = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName  = NULL;
		wcex.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"Sierpinski";

		RegisterClassEx(&wcex);


		// Because the CreateWindow function takes its size in pixels,
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);


		// Create the window.
		/*_hwnd = CreateWindow(L"SierpinskiApp", L"SierpinskiApp", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, static_cast<UINT>(ceil(640.f * dpiX / 96.f)), static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
		NULL, NULL, HINST_THISCOMPONENT, this);*/
		_hwnd = CreateWindow(
			L"D2DDemoApp",
			L"Direct2D Demo App",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
			static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
			);
		hr = _hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(_hwnd, SW_SHOWNORMAL);
			UpdateWindow(_hwnd);
		}
	}

	return hr;
}

// Process and dispatch messages
void BasicApp::RunMessageLoop(){
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// Initialize device-independent resources.
HRESULT BasicApp::CreateDeviceIndependentResources(){
	HRESULT hr = S_OK;

	// create direct2d factory
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_pDirect2dFactory);
	return hr;
}

// Initialize device-dependent resources.
HRESULT BasicApp::CreateDeviceResources(){
	HRESULT hr = S_OK;
	if(!_pRenderTarget){
		RECT rc;
		GetClientRect(_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// Create render target
		hr = _pDirect2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(_hwnd, size), &_pRenderTarget);

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = _pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&_pLightSlateGrayBrush
				);
		}
		if (SUCCEEDED(hr))
		{
			// Create a blue brush.
			hr = _pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&_pCornflowerBlueBrush
				);
		}
	}
	return hr;
}

// Release device-dependent resource.
void BasicApp::DiscardDeviceResources(){
	SafeRelease(&_pRenderTarget);
	SafeRelease(&_pLightSlateGrayBrush);
	SafeRelease(&_pCornflowerBlueBrush);
}

// Draw content.
HRESULT BasicApp::OnRender(){
	HRESULT hr = S_OK;

	hr = CreateDeviceResources();
	if(SUCCEEDED(hr)){
		// Start drawing
		_pRenderTarget->BeginDraw();
		// Set identity transform
		_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		// clear the drawing area
		_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
		D2D1_SIZE_F rtSize = _pRenderTarget->GetSize();
		// Draw a grid background.
		int width = static_cast<int>(rtSize.width);
		int height = static_cast<int>(rtSize.height);

		for (int x = 0; x < width; x += 10)
		{
			_pRenderTarget->DrawLine(
				D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
				D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
				_pLightSlateGrayBrush,
				0.5f
				);
		}

		for (int y = 0; y < height; y += 10)
		{
			_pRenderTarget->DrawLine(
				D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
				D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
				_pLightSlateGrayBrush,
				0.5f
				);
		}

		// Draw two rectangles.
		D2D1_RECT_F rectangle1 = D2D1::RectF(
			rtSize.width/2 - 50.0f,
			rtSize.height/2 - 50.0f,
			rtSize.width/2 + 50.0f,
			rtSize.height/2 + 50.0f
			);

		D2D1_RECT_F rectangle2 = D2D1::RectF(
			rtSize.width/2 - 100.0f,
			rtSize.height/2 - 100.0f,
			rtSize.width/2 + 100.0f,
			rtSize.height/2 + 100.0f
			);
		// Draw the first filled rectangle.
		_pRenderTarget->FillRectangle(&rectangle1, _pLightSlateGrayBrush);
		// Draw the second filled rectangle.
		_pRenderTarget->FillRectangle(&rectangle2, _pCornflowerBlueBrush);
		hr = _pRenderTarget->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}
	}
	return hr;
}

// Resize the render target.
void BasicApp::OnResize(UINT width, UINT height){
	if (_pRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here, because the error will be returned again
		// the next time EndDraw is called.
		_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}

// The windows procedure.
LRESULT CALLBACK BasicApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		BasicApp *app = (BasicApp *)pcs->lpCreateParams;

		::SetWindowLongPtrW(hWnd,GWLP_USERDATA,PtrToUlong(app));

		result = 1;
	}
	else
	{
		BasicApp *app = reinterpret_cast<BasicApp *>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));

		bool wasHandled = false;

		if (app)
		{
			switch (message)
			{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					app->OnResize(width, height);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_DISPLAYCHANGE:
				{
					InvalidateRect(hWnd, NULL, FALSE);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_PAINT:
				{
					app->OnRender();
					ValidateRect(hWnd, NULL);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_DESTROY:
				{
					PostQuitMessage(0);
				}
				result = 1;
				wasHandled = true;
				break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	return result;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow){
	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap used
	// by the process.
	// The return value is ignored, because we want to continue running in the
	// unlikely event that HeapSetInformation fails.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		BasicApp app;

		if (SUCCEEDED(app.Initialize()))
		{
			app.RunMessageLoop();
		}
		CoUninitialize();
	}
	return 0;
}