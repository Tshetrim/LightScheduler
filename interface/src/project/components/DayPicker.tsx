import React, { useState } from "react";
import { Button, Box } from "@mui/material";

const daysOfWeek = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"];

interface DayPickerProps {
	activeDays: string[]; // Initially active days
	onChange: (days: string[]) => void; // Callback for when the active days change
}

const DayPicker: React.FC<DayPickerProps> = ({ activeDays: initialActiveDays, onChange }) => {
	const [activeDays, setActiveDays] = useState<string[]>(initialActiveDays);

	const toggleDay = (day: string) => {
		const currentIndex = activeDays.indexOf(day);
		const newActiveDays = [...activeDays];

		if (currentIndex === -1) {
			newActiveDays.push(day);
		} else {
			newActiveDays.splice(currentIndex, 1);
		}

		setActiveDays(newActiveDays);
		onChange(newActiveDays);
	};

	const selectAllDays = () => {
		setActiveDays(daysOfWeek);
		onChange(daysOfWeek);
	};

	const clearAllDays = () => {
		setActiveDays([]);
		onChange([]);
	};

	return (
		<Box display="flex" justifyContent="center" flexWrap="wrap" sx={{ width: "100%" }}>
			{daysOfWeek.map((fullDay, index) => (
				<Button
					key={fullDay}
					variant={activeDays.includes(fullDay) ? "contained" : "outlined"}
					onClick={() => toggleDay(fullDay)}
					sx={{
						flexGrow: 1,
						flexBasis: { xs: 0, md: "auto" },
						minWidth: { xs: "14%", md: "auto" },
						my: 1,
						border: "2px solid",
						borderColor: activeDays.includes(fullDay) ? "transparent" : "divider",
						fontSize: { xs: "0.75rem", sm: "1rem" },
						borderRadius:
							index === 0 ? "15px 0 0 15px" : index === daysOfWeek.length - 1 ? "0 15px 15px 0" : "0",
						"&:not(:last-child)": {
							borderRight: "1px solid",
							borderColor: "divider",
						},
						padding: "12px 10px",
					}}
				>
					{fullDay.substring(0, 3)}
				</Button>
			))}
			<Button variant="contained" color="warning" onClick={selectAllDays} sx={{ mx: 0.5, my: 1, flexGrow: 1 }}>
				Daily
			</Button>
			<Button variant="outlined" color="error" onClick={clearAllDays} sx={{ mx: 0.5, my: 1, flexGrow: 1 }}>
				Clear
			</Button>
		</Box>
	);
};

export default DayPicker;
