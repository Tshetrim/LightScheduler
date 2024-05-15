import { FC, useContext, useState, useEffect, useMemo, useRef } from "react";

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
import AutomaticOfflineTimeSet from "./components/AutomaticOfflineTimeSet";
import { v4 as uuidv4 } from "uuid";

import { AuthenticationContext } from "../contexts/authentication";

interface ScheduleWithId extends Schedule {
	id: string;
}
const RGBLightStateRestForm: FC = () => {
	const { loadData, saveData, saving, setData, data, errorMessage } = useRest<RGBLightState>({
		read: RGBLightAPI.readRGBLightState,
		update: RGBLightAPI.updateRGBLightState,
	});
	const { me } = useContext(AuthenticationContext);
	const idsRef = useRef<Map<number, string>>(new Map());

	useEffect(() => {
		const scheduleCount = data?.schedules.length || 0;
		const currentIds = idsRef.current;
		const newIds = new Map(currentIds);

		// Assign new IDs for new schedules
		for (let i = 0; i < scheduleCount; i++) {
			if (!currentIds.has(i)) {
				newIds.set(i, uuidv4());
			}
		}

		// Remove old IDs
		currentIds.forEach((id, key) => {
			console.log("id:", id, "key:", key);

			if (key >= scheduleCount) {
				newIds.delete(key);
			}
		});

		idsRef.current = newIds;
		console.log(idsRef);
	}, [data?.schedules.length]);

	const schedulesWithIds: ScheduleWithId[] = useMemo(() => {
		if (!data?.schedules) return [];
		// Directly use ids from idsRef
		console.log(idsRef.current);
		return data.schedules.map((schedule, index) => {
			const id = idsRef.current.get(index);
			return {
				...schedule,
				id: id || uuidv4(), // Fallback to generating an ID if not found, though it should ideally always be found
			};
		});
	}, [data?.schedules]);

	const handleScheduleChange = (scheduleId: string) => (newScheduleData: Partial<Schedule>) => {
		setData((prevData) => {
			if (!prevData) return undefined;
			const updatedSchedules = schedulesWithIds.map((schedule) =>
				schedule.id === scheduleId ? { ...schedule, ...newScheduleData } : schedule
			);
			return {
				...prevData,
				schedules: updatedSchedules.map((schedule) => ({
					start: schedule.start,
					end: schedule.end,
					color: schedule.color,
					daysActive: schedule.daysActive,
				})),
			};
		});
	};

	const handleAddSchedule = () => {
		const newSchedule: ScheduleWithId = {
			id: uuidv4(), // Directly assign an ID here
			start: localDateTimeToEpoch(new Date()),
			end: localDateTimeToEpoch(new Date()),
			color: { r: 0, g: 0, b: 0 },
			daysActive: [],
		};
		setData((prevData) => {
			if (!prevData) return undefined;
			return { ...prevData, schedules: [...schedulesWithIds, newSchedule] };
		});
	};

	const handleRemoveSchedule = (scheduleId: string) => () => {
		setData((prevData) => {
			if (!prevData) return undefined;
			return {
				...prevData,
				schedules: schedulesWithIds.filter((schedule) => schedule.id !== scheduleId),
			};
		});
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

		// console.log(data);

		return (
			<>
				<AutomaticOfflineTimeSet></AutomaticOfflineTimeSet>

				<Typography variant="h6">Schedules</Typography>
				<Divider sx={{ mb: 2 }} />
				<MessageBox
					level="info"
					message="If any day is toggled, date will be ignored and the schedule will retrigger on that day and time"
					my={2}
				/>

				{schedulesWithIds &&
					schedulesWithIds.map((schedule) => (
						<ScheduleItem
							key={schedule.id}
							schedule={schedule}
							onChange={handleScheduleChange(schedule.id)}
							onRemove={handleRemoveSchedule(schedule.id)}
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
										onChange={(e) =>
											handlePinsChange(pinKey as keyof RGBPins, parseInt(e.target.value))
										}
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
		<SectionContent title="" titleGutter>
			{content()}
		</SectionContent>
	);
};

export default RGBLightStateRestForm;
