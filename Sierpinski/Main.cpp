// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "BasicApp.h"

// Provides the application entry point.
int WINAPI WinMain(
    HINSTANCE /* hInstance */,
    HINSTANCE /* hPrevInstance */,
    LPSTR /* lpCmdLine */,
    int /* nCmdShow */
    )
{
    // Use HeapSetInformation to specify that the process should
    // terminate if the heap manager detects an error in any heap used
    // by the process.
    // The return value is ignored, because we want to continue running in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            BasicApp app;

            if (SUCCEEDED(app.Initialize()))
            {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}

// DemoApp constructor
BasicApp::BasicApp() :
    _hwnd(NULL),
    _pDirect2dFactory(NULL),
    _pRenderTarget(NULL),
    _pPointBrush(NULL),
    _pLineBrush(NULL),
	_numChaoticPoints(256)
{
}

// DemoApp destructor
// Releases the application's resources.
BasicApp::~BasicApp()
{
    SafeRelease(&_pDirect2dFactory);
    SafeRelease(&_pRenderTarget);
    SafeRelease(&_pPointBrush);
    SafeRelease(&_pLineBrush);

}

// Creates the application window and device-independent
// resources.
HRESULT BasicApp::Initialize()
{
    HRESULT hr;

    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    hr = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr))
    {
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
        wcex.lpszClassName = L"SierpinskiTriangle";

        RegisterClassEx(&wcex);


        // Because the CreateWindow function takes its size in pixels,
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;

        // The factory returns the current system DPI. This is also the value it will use
        // to create its own windows.
        _pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);


        // Create the window.
        _hwnd = CreateWindow(
            L"SierpinskiTriangle",
            L"Sierpinski Triangle",
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

// Creates resources that are not bound to a particular device.
// Their lifetime effectively extends for the duration of the
// application.
HRESULT BasicApp::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_pDirect2dFactory);

    return hr;
}


// Creates resources that are bound to a particular
// Direct3D device. These resources need to be recreated
// if the Direct3D device dissapears, such as when the display
// changes, the window is remoted, etc.
HRESULT BasicApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!_pRenderTarget)
    {
        RECT rc;
        GetClientRect(_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );

        // Create a Direct2D render target.
        hr = _pDirect2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(_hwnd, size),
            &_pRenderTarget
            );


        if (SUCCEEDED(hr))
        {
            // Create a gray brush.
            hr = _pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::DarkGreen),
                &_pPointBrush
                );
        }
        if (SUCCEEDED(hr))
        {
            // Create a blue brush.
            hr = _pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                &_pLineBrush
                );
        }
    }

    return hr;
}

// Discards device-dependent resources. These resources must be
// recreated when the Direct3D device is lost.
void BasicApp::DiscardDeviceResources()
{
    SafeRelease(&_pRenderTarget);
    SafeRelease(&_pPointBrush);
    SafeRelease(&_pLineBrush);
}

// Runs the main window message loop.
void BasicApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void BasicApp::DrawPoint(D2D1_POINT_2F center, ID2D1SolidColorBrush* brush, int offset){
	_pRenderTarget->DrawLine(
            D2D1::Point2F(center.x-offset, center.y),
            D2D1::Point2F(center.x+offset, center.y),
            brush,
            1.0f);
	_pRenderTarget->DrawLine(
            D2D1::Point2F(center.x, center.y-offset),
            D2D1::Point2F(center.x, center.y+offset),
            brush,
            1.0f);
}

D2D1_POINT_2F BasicApp::CalculateMidpoint(D2D1_POINT_2F first, D2D1_POINT_2F second){
	return D2D1::Point2F((first.x + second.x) / 2, (first.y + second.y) / 2 );
}

// This method discards device-specific
// resources if the Direct3D device dissapears during execution and
// recreates the resources the next time it's invoked.
HRESULT BasicApp::OnRender()
{
    HRESULT hr = S_OK;

    hr = CreateDeviceResources();

    if (SUCCEEDED(hr))
    {
        _pRenderTarget->BeginDraw();

        _pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        _pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        D2D1_SIZE_F rtSize = _pRenderTarget->GetSize();

		for(int i = 0; i < _points.size(); i++){
			DrawPoint(_points[i], _pPointBrush, 4);
		}
		if(_points.size() == 4){
			auto seedPoint = _points[3];
			for (int i = 0; i < _numChaoticPoints; i++) {
				// choose a random vertex of the triangle
				auto chosenVertex = _points[rand() % 3];
				// Draw the midpoint of the random vertex and the seedpoint
				// Then make that midpoint the seedpoint for the next iteration
				auto midpoint = CalculateMidpoint(chosenVertex, seedPoint);
				DrawPoint(midpoint, _pPointBrush, 2);
				seedPoint = midpoint;
			}
		}
        hr = _pRenderTarget->EndDraw();
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}

//  If the application receives a WM_SIZE message, this method
//  resizes the render target appropriately.
void BasicApp::OnResize(UINT width, UINT height)
{
    if (_pRenderTarget)
    {
        // Note: This method can fail, but it's okay to ignore the
        // error here, because the error will be returned again
        // the next time EndDraw is called.
        _pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

void BasicApp::OnLButtonUp(int pixelX, int pixelY, DWORD flags)
{
    //const float dipX = DPIScale::PixelsToDipsX(pixelX);
    //const float dipY = DPIScale::PixelsToDipsY(pixelY);
	if (_points.size() < 4) {
		_points.push_back(D2D1::Point2F(pixelX, pixelY));
	}
    InvalidateRect(_hwnd, NULL, FALSE);
}

void BasicApp::OnKeyDown(UINT vkey)
{
	bool needRedraw = false;
    switch (vkey)
    {
	case 78: // n
		_points.push_back(D2D1::Point2F(200, 200));
		// redraw
	case 85: // u
		if (_numChaoticPoints < 1048576) {
			_numChaoticPoints <<= 1; // * 2
		}
		break;
	case 68: // d
		if (_numChaoticPoints > 256) {
			_numChaoticPoints >>= 1; // / 2
		}
		break;
	case 67: // d
		_points.clear();
		break;

	default:
		break;
	}
	InvalidateRect(_hwnd, NULL, FALSE);
}


// Handles window messages.
LRESULT CALLBACK BasicApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        BasicApp *pDemoApp = (BasicApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pDemoApp));

        result = 1;
    }
    else
    {
        BasicApp *pDemoApp = reinterpret_cast<BasicApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
                )));

        bool wasHandled = false;

        if (pDemoApp)
        {
            switch (message)
            {
            case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pDemoApp->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_PAINT:
                {
                    pDemoApp->OnRender();
                    ValidateRect(hwnd, NULL);
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
			// left click
			case WM_LBUTTONUP:
				pDemoApp->OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
				wasHandled = true;
				break;
			// keyboard key press
			case WM_KEYDOWN:
				pDemoApp->OnKeyDown((UINT)wParam);
				wasHandled = true;
				return 0;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}