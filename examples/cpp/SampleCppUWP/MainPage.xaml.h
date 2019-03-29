//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

#include <future>
#include <thread>
#include <algorithm>

namespace SampleCppUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
        static MainPage^ self;

    public:

        static MainPage^ Self()
        {
            return self;
        }

        MainPage();

        void Print(Platform::String^ text);
        void ScrollToEnd();

    private:
        void PerfTestClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void SimpleTestClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        std::future<void> perfTestResult;
    };
}
