import { FC, useState } from "react";
import { TextField } from "@mui/material";
import { RGBColor } from "../types";

interface RGBColorProps {
	color: RGBColor;
	onChange: (newColor: RGBColor) => void;
}

const RGBColorPicker: FC<RGBColorProps> = ({ color, onChange }) => {
	const [localColor, setLocalColor] = useState(
		`#${color.r.toString(16).padStart(2, "0")}${color.g.toString(16).padStart(2, "0")}${color.b
			.toString(16)
			.padStart(2, "0")}`
	);

	// Handle real-time changes locally
	const handleColorChange = (event: React.ChangeEvent<HTMLInputElement>) => {
		setLocalColor(event.target.value); // Update locally only
	};

	// Commit changes to external state when the user stops interacting
	const handleColorChangeCommit = (event: React.FocusEvent<HTMLInputElement>) => {
		const hex = event.target.value;
		const newColor = {
			r: parseInt(hex.slice(1, 3), 16),
			g: parseInt(hex.slice(3, 5), 16),
			b: parseInt(hex.slice(5, 7), 16),
		};
		onChange(newColor);
	};

	return (
		<TextField
			fullWidth
			label="RGB Color"
			type="color"
			value={localColor}
			onChange={handleColorChange}
			onBlur={handleColorChangeCommit}
			variant="outlined"
		/>
	);
};

export default RGBColorPicker;
