import { FC, useMemo } from "react";
import { TextField, Typography, Box, Paper, IconButton, Grid, Tooltip } from "@mui/material";
import DeleteIcon from "@mui/icons-material/Delete";
import { localDateTimeToEpoch, epochToLocalDateTime, isEpochTimePast, isEpochTimeActive } from "../../utils";
import RGBColorPicker from "./RGBColorPicker";
import { RGBColor, Schedule } from "../types";

interface ScheduleItemProps {
	schedule: Schedule;
	onChange: (newSchedule: Partial<Schedule>) => void;
	onRemove: () => void;
}

const ScheduleItem: FC<ScheduleItemProps> = ({ schedule, onChange, onRemove }) => {
	const handleTimeChange = (field: "start" | "end") => (event: React.ChangeEvent<HTMLInputElement>) => {
		onChange({ ...schedule, [field]: localDateTimeToEpoch(event.target.value) });
	};

	const handleColorChange = (newColor: RGBColor) => {
		onChange({ ...schedule, color: newColor });
	};

	const isPast = useMemo(() => isEpochTimePast(schedule.end), [schedule.end]);
	const isActive = useMemo(() => isEpochTimeActive(schedule.start, schedule.end), [schedule.start, schedule.end]);

	const paperStyle = useMemo(
		() => ({
			p: 2,
			mb: 2,
			backgroundColor: isActive ? "#e8f5e9" : isPast ? "#f0f0f0" : "inherit",
			border: isActive ? "2px solid #4caf50" : isPast ? "5px solid #e0e0e0" : "2px solid #e0e0e0",
			cursor: "pointer",
			"& input": {
				cursor: "pointer",
			},
		}),
		[isPast, isActive]
	);

	return (
		<Tooltip
			title={isPast ? "🔴 Schedule is past date and stale 🔴" : isActive ? "🟢 Currently Active 🟢" : "⏰ To be Active ⏰"}
			placement="top"
		>
			<Paper variant="outlined" sx={paperStyle}>
				<Grid container spacing={2} alignItems="center">
					<Grid item xs={12} sm={6}>
						<TextField
							fullWidth
							label="Start Time"
							type="datetime-local"
							value={epochToLocalDateTime(schedule.start)}
							onChange={handleTimeChange("start")}
							InputLabelProps={{ shrink: true }}
							variant="outlined"
						/>
					</Grid>
					<Grid item xs={12} sm={6}>
						<TextField
							fullWidth
							label="End Time"
							type="datetime-local"
							value={epochToLocalDateTime(schedule.end)}
							onChange={handleTimeChange("end")}
							InputLabelProps={{ shrink: true }}
							variant="outlined"
						/>
					</Grid>
					<Grid item xs={12} sm={6}>
						<RGBColorPicker color={schedule.color} onChange={handleColorChange} />
					</Grid>
					<Grid item xs={12} sm={6}>
						<IconButton aria-label="delete" onClick={onRemove}>
							<DeleteIcon />
						</IconButton>
					</Grid>
				</Grid>
			</Paper>
		</Tooltip>
	);
};

export default ScheduleItem;