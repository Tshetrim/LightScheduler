import { AxiosPromise } from "axios";

import { AXIOS } from "../api/endpoints";
import { RGBLightState } from "./types";

export function readRGBLightState(): AxiosPromise<RGBLightState> {
	return AXIOS.get("/rgbLightState");
}

export function updateRGBLightState(lightState: RGBLightState): AxiosPromise<RGBLightState> {
	return AXIOS.post("/rgbLightState", lightState);
}
