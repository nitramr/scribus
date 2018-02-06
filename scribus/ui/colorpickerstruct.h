#ifndef COLORPICKERSTRUCT_H
#define COLORPICKERSTRUCT_H

enum ColorPaintMode {
	Solid = 0,
	Gradient = 1,
	Hatch = 2,
	Pattern = 3
};

enum ObjectPaintMode {
	Fill = 0,
	Stroke = 1

};

enum GradientTypes {
	Linear = 0,
	Radial = 1,
	Conical = 2,
	FourColors = 3,
	Diamond = 4,
	Mesh = 5,
	PatchMesh = 6
};

#endif // COLORPICKERSTRUCT_H
