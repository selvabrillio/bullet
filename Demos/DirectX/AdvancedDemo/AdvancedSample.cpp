//--------------------------------------------------------------------------------------
// Copyright © Microsoft Corporation. All rights reserved.
// Portions Copyright © Microsoft Open Technologies, Inc. All rights reserved.
// Microsoft Public License (Ms-PL)
//
// This license governs use of the accompanying software. If you use the software, you accept this license. If you do not accept the license, do not use the software.
//
// 1. Definitions
//
// The terms "reproduce," "reproduction," "derivative works," and "distribution" have the same meaning here as under U.S. copyright law.
//
// A "contribution" is the original software, or any additions or changes to the software.
//
// A "contributor" is any person that distributes its contribution under this license.
//
// "Licensed patents" are a contributor's patent claims that read directly on its contribution.
//
// 2. Grant of Rights
//
// (A) Copyright Grant- Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non-exclusive, worldwide, royalty-free copyright license to reproduce its contribution, prepare derivative works of its contribution, and distribute its contribution or any derivative works that you create.
//
// (B) Patent Grant- Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non-exclusive, worldwide, royalty-free license under its licensed patents to make, have made, use, sell, offer for sale, import, and/or otherwise dispose of its contribution in the software or derivative works of the contribution in the software.
// 
// 3. Conditions and Limitations
// 
// (A) No Trademark License- This license does not grant you rights to use any contributors' name, logo, or trademarks.
// 
// (B) If you bring a patent claim against any contributor over patents that you claim are infringed by the software, your patent license from such contributor to the software ends automatically.
// 
// (C) If you distribute any portion of the software, you must retain all copyright, patent, trademark, and attribution notices that are present in the software.
// 
// (D) If you distribute any portion of the software in source code form, you may do so only under this license by including a complete copy of this license with your distribution. If you distribute any portion of the software in compiled or object code form, you may only do so under a license that complies with this license.
// 
// (E) The software is licensed "as-is." You bear the risk of using it. The contributors give no express warranties, guarantees or conditions. You may have additional consumer rights under your local laws which this license cannot change. To the extent permitted under your local laws, the contributors exclude the implied warranties of merchantability, fitness for a particular purpose and non-infringement.
//--------------------------------------------------------------------------------------
#include "pch.h"

#include "AdvancedSample.h"
#include "BasicTimer.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace concurrency;

AdvancedSample::AdvancedSample() :
	m_windowClosed(false),
	m_windowVisible(true),
	level(std::unique_ptr<Level>(new Level()))
{
}

void AdvancedSample::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &AdvancedSample::OnActivated);

	CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &AdvancedSample::OnSuspending);

	CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &AdvancedSample::OnResuming);

	renderer = ref new Renderer();
}

void AdvancedSample::SetWindow(CoreWindow^ window)
{
	this->window = window;
	window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &AdvancedSample::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &AdvancedSample::OnVisibilityChanged);

	window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &AdvancedSample::OnWindowClosed);

	window->PointerCursor = ref new CoreCursor(CoreCursorType::UniversalNo, 0);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &AdvancedSample::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &AdvancedSample::OnPointerMoved);

	renderer->Initialize(CoreWindow::GetForCurrentThread());
	level->Initialize(renderer);
}

void AdvancedSample::Load(Platform::String^ entryPoint)
{
}

void AdvancedSample::Run()
{
	BasicTimer^ timer = ref new BasicTimer();

	VirtualKey key = VirtualKey::None;

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			if (this->window->GetAsyncKeyState(VirtualKey::Space) == CoreVirtualKeyStates::Down)
			{
				level->Fire(renderer);
			}
			if (this->window->GetAsyncKeyState(VirtualKey::W) == CoreVirtualKeyStates::Down)
			{
				key = VirtualKey::W;
			}
			if (this->window->GetAsyncKeyState(VirtualKey::S) == CoreVirtualKeyStates::Down)
			{
				key = VirtualKey::S;
			}
			if (this->window->GetAsyncKeyState(VirtualKey::D) == CoreVirtualKeyStates::Down)
			{
				key = VirtualKey::D;
			}
			if (this->window->GetAsyncKeyState(VirtualKey::A) == CoreVirtualKeyStates::Down)
			{
				key = VirtualKey::A;
			}
			if (this->window->GetAsyncKeyState(VirtualKey::Left) == CoreVirtualKeyStates::Down)
			{
				key = VirtualKey::Left;
			}
			if (this->window->GetAsyncKeyState(VirtualKey::Right) == CoreVirtualKeyStates::Down)
			{
				key = VirtualKey::Right;
			}

			if (this->window->GetAsyncKeyState(VirtualKey::Up) == CoreVirtualKeyStates::Down)
			{
				key = VirtualKey::Up;
			}
			if (this->window->GetAsyncKeyState(VirtualKey::Down) == CoreVirtualKeyStates::Down)
			{
				key = VirtualKey::Down;
			}

			timer->Update();

			level->Update();
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			renderer->Update(timer->Total, timer->Delta, key);
			renderer->Render();
			renderer->Present(); // This call is synchronized to the display frame rate.

			key = VirtualKey::None;
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void AdvancedSample::Uninitialize()
{
}

void AdvancedSample::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	renderer->UpdateForWindowSizeChange();
}

void AdvancedSample::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void AdvancedSample::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void AdvancedSample::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void AdvancedSample::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
}

void AdvancedSample::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void AdvancedSample::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// Insert your code here.

		deferral->Complete();
	});
}
 
void AdvancedSample::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new AdvancedSample();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}
