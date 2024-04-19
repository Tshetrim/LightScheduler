export interface RGBPins {
	rPin: number;
	gPin: number;
	bPin: number;
}

export interface RGBColor {
	r: number;
	g: number;
	b: number;
}

export interface Schedule {
	start: number;
	end: number;
	color: RGBColor;
}

export interface Schedules {
	schedules: Schedule[];
}

export interface RGBLightState {
	pins: RGBPins;
	color: RGBColor;
	schedules: Schedule[];
}
