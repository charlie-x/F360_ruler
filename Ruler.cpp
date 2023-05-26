
#include <Core/CoreAll.h>
#include <Fusion/FusionAll.h>
#include <CAM/CAMAll.h>
#include <stdexcept>
#include <string>

using namespace adsk::core;
using namespace adsk::fusion;
using namespace adsk::cam;

Ptr<Application> app;
Ptr<UserInterface> ui;


void add_ruler_pattern() {
	// Get the active document and root component
	Ptr<Application> app = Application::get();
	if (!app)
		return;
	Ptr<Product> doc = app->activeProduct();
	
	if (!doc)
		return;

	Ptr<Design> design = app->activeProduct();
	if (!design)
		return;

	Ptr<Component> root_comp = design->rootComponent();
	if (!root_comp)
		return;

	// Prompt the user to enter parameters
	Ptr<UserInterface> ui = app->userInterface();
	ui->messageBox("Select a plane for the ruler");
	Ptr<Selection> plane_input = ui->selectEntity("Select a sketch plane", "PlanarFaces");
	if (!plane_input) {
		ui->messageBox("No plane selected.");
		return;
	}
	Ptr<Base> plane = plane_input->entity();
	if (!plane) {
		ui->messageBox("No plane available.");
		return;
	}

	bool cancelled = false;
	std::string rulerLength = ui->inputBox("enter the total length for the ruler in mm", cancelled, "ruler length", "100");
	if (cancelled == true) {
		return;
	}
	std::string cmLineLength = ui->inputBox("enter the length for the cm lines:", cancelled, "cm line length", "20");
	if (cancelled == true) {
		return;
	}
	std::string mmLineLength = ui->inputBox("enter the length for the mm lines:", cancelled, "mm line length", "14");
	if (cancelled == true) {
		return;
	}
	std::string textSize = ui->inputBox("enter the desired text size:", cancelled, "text size", "10");
	if (cancelled == true) {
		return;
	}

	// Check if the user canceled the input
	if (cmLineLength == "cancel" || mmLineLength == "cancel" || rulerLength == "cancel" || textSize == "cancel") {
		ui->messageBox("User Cancelled.");
		return;
	}

	// Convert user input to appropriate data types
	int rulerLengthVal = std::stoi(rulerLength);
	float cmLineLengthVal = std::stof(cmLineLength);
	float mmLineLengthVal = std::stof(mmLineLength);
	float textSizeVal = std::stof(textSize) / 10;

	// Create a new sketch
	Ptr<Sketches> sketches = root_comp->sketches();
	Ptr<Sketch> sketch = sketches->add(plane);

	// Loop to create lines and numbers
	for (int i = 0; i <= rulerLengthVal; i++) {
		bool isCMValue = i % 10 == 0;
		Ptr<Point3D> start_point = adsk::core::Point3D::create(i / 10.0f, 0, 0);
		Ptr<Point3D> end_point = adsk::core::Point3D::create(i / 10.0f, isCMValue ? cmLineLengthVal / 10.0f : mmLineLengthVal / 10.0f, 0);
		sketch->sketchCurves()->sketchLines()->addByTwoPoints(start_point, end_point);
		if (isCMValue) {
			float boxSize = 0.8f;
			float yMinPosition = (cmLineLengthVal + 2) / 10.0f;
			float yMaxPosition = yMinPosition + textSizeVal;
			float xCenterPosition = i / 10.0f;
			float xMinPosition = xCenterPosition - boxSize / 2;
			float xMaxPosition = xCenterPosition + boxSize / 2;
			std::string textTitle = std::to_string(i);
			try {
				Ptr<SketchTextInput> textInput = sketch->sketchTexts()->createInput2(textTitle, textSizeVal);
				textInput->setAsMultiLine(Point3D::create(xMinPosition, yMinPosition, 0),
					Point3D::create(xMaxPosition, yMaxPosition, 0),
					HorizontalAlignments::CenterHorizontalAlignment,
					VerticalAlignments::BottomVerticalAlignment, 0);
				// Add the text to the sketch
				Ptr<SketchText> text = sketch->sketchTexts()->add(textInput);
			}
			catch (std::exception& e) {
				ui->messageBox(e.what());
			}
		}
	}
	// Exit sketch mode
	sketch->isVisible(false);


}

extern "C" XI_EXPORT bool run(const char* context)
{
	app = Application::get();
	if (!app)
		return false;

	ui = app->userInterface();
	if (!ui)
		return false;

	add_ruler_pattern();
	
	// add in versus script..
	adsk::terminate();

	return true;
}

extern "C" XI_EXPORT bool stop(const char* context)
{
	if (ui)
	{
		ui = nullptr;
	}

	return true;
}


#ifdef XI_WIN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hmodule, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif // XI_WIN
