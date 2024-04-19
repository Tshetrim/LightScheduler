import { FC, useContext } from "react";

import { Button, TextField, Typography, Box, Paper, IconButton, Grid, Divider, Checkbox, Tooltip } from "@mui/material";
import SaveIcon from "@mui/icons-material/Save";
import DeleteIcon from "@mui/icons-material/Delete";
import AddCircleOutlineIcon from "@mui/icons-material/AddCircleOutline";

import { SectionContent, FormLoader, BlockFormControlLabel, ButtonRow, MessageBox } from "../components";
import { updateValue, useRest } from "../utils";

import * as RGBLightAPI from "./api";
import { RGBPins, RGBColor, Schedule, Schedules, RGBLightState } from "./types";
import { epochToLocalDateTime, localDateTimeToEpoch, isEpochTimePast, isEpochTimeActive } from "../utils";

import RGBColorPicker from "./components/RGBColorPicker";
import ScheduleItem from "./components/ScheduleItem";

import { AuthenticationContext } from "../contexts/authentication";

const RGBLightStateRestForm: FC = () => {
	const { loadData, saveData, saving, setData, data, errorMessage } = useRest<RGBLightState>({
		read: RGBLightAPI.readRGBLightState,
		update: RGBLightAPI.updateRGBLightState,
	});
	const { me } = useContext(AuthenticationContext);

	const handleScheduleChange = (index: number) => (newSchedule: Partial<Schedule>) => {
		if (data) {
			const newSchedules = [...data.schedules];
			newSchedules[index] = { ...newSchedules[index], ...newSchedule };
			setData({ ...data, schedules: newSchedules });
		}
	};

	const handleAddSchedule = () => {
		const newSchedule: Schedule = {
			start: localDateTimeToEpoch(new Date()),
			end: localDateTimeToEpoch(new Date()),
			color: { r: 0, g: 0, b: 0 },
		};
		if (data) {
			setData({ ...data, schedules: [...data.schedules, newSchedule] });
		}
	};

	const handleRemoveSchedule = (index: number) => () => {
		if (data) {
			const newSchedules = [...data.schedules];
			newSchedules.splice(index, 1);
			setData({ ...data, schedules: newSchedules });
		}
	};

	function updateLightState<Key extends keyof RGBLightState>(key: Key, newValue: RGBLightState[Key]) {
		setData((prevData) => {
			if (!prevData) return prevData;
			return {
				...prevData,
				[key]: newValue,
			};
		});
	}

	const handleColorChange = (newColor: RGBColor) => {
		updateLightState("color", newColor);
	};

	const handlePinsChange = (pin: keyof RGBPins, newValue: number) => {
		if (!data) return;
		const newPins = {
			...data.pins,
			[pin]: newValue,
		};
		updateLightState("pins", newPins);
	};

	const content = () => {
		if (!data) {
			return <FormLoader onRetry={loadData} errorMessage={errorMessage} />;
		}

		console.log(data);

		return (
			<>
				<MessageBox level="info" message="Below is the settings for the light" my={2} />

				<Typography variant="h6">Light Settings</Typography>
				<Divider sx={{ mb: 2 }} />

				{data.schedules.map((schedule, index) => (
					<ScheduleItem
						key={index}
						schedule={schedule}
						onChange={handleScheduleChange(index)}
						onRemove={() => handleRemoveSchedule(index)}
					/>
				))}

				<Box display="flex" justifyContent="flex-end" mt={2}>
					<IconButton color="primary" onClick={handleAddSchedule}>
						<AddCircleOutlineIcon />
					</IconButton>
				</Box>

				<Box mt={4}>
					<Typography variant="h6" gutterBottom>
						Change Color Settings
					</Typography>
					<RGBColorPicker color={data.color} onChange={handleColorChange} />
				</Box>

				{me?.admin && (
					<Box mt={4}>
						<Typography variant="h6" gutterBottom>
							Pin Settings
						</Typography>
						<Grid container spacing={2}>
							{Object.entries(data.pins).map(([pinKey, pinValue]) => (
								<Grid item xs={4} key={pinKey}>
									<TextField
										fullWidth
										label={`${pinKey.toUpperCase()} Pin`}
										type="number"
										inputProps={{ min: 0, max: 255 }}
										value={pinValue}
										onChange={(e) => handlePinsChange(pinKey as keyof RGBPins, parseInt(e.target.value))}
										variant="outlined"
									/>
								</Grid>
							))}
						</Grid>
					</Box>
				)}

				<ButtonRow mt={2}>
					<Button
						startIcon={<SaveIcon />}
						disabled={saving}
						variant="contained"
						color="primary"
						type="submit"
						onClick={saveData}
					>
						Save
					</Button>
				</ButtonRow>
				{errorMessage && <MessageBox level="error" message={errorMessage} mt={2} />}
			</>
		);
	};

	return (
		<SectionContent title="Auto Light" titleGutter>
			{content()}
		</SectionContent>
	);
};

export default RGBLightStateRestForm;
