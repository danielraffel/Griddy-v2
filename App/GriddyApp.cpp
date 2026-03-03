#include <JuceHeader.h>
#include "GriddyAppMainComponent.h"

class GriddyAppApplication : public juce::JUCEApplication
{
public:
    GriddyAppApplication() = default;

    const juce::String getApplicationName() override { return "Griddy"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow_ = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow_.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String&) override
    {
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(juce::String name)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel()
                                 .findColour(juce::ResizableWindow::backgroundColourId),
                             DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new GriddyAppMainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif
            setVisible(true);

#if JUCE_IOS || JUCE_ANDROID
            // Use 'this' (MainWindow) — it has the native peer.
            // Using getContentComponent() fails the jassert at juce_Desktop.cpp:326
            // because child components don't have their own ComponentPeer.
            juce::MessageManager::callAsync([this]
            {
                juce::Desktop::getInstance().setKioskModeComponent(this, false);
            });
#endif
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

    std::unique_ptr<MainWindow> mainWindow_;
};

START_JUCE_APPLICATION(GriddyAppApplication)
