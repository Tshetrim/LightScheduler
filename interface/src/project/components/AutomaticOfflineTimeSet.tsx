import { FC, useEffect, useState, useCallback, useRef } from "react";
import { useSnackbar } from "notistack";
import { Box, Typography } from "@mui/material";
import * as NTPApi from "../../api/ntp";
import { NTPStatus, NTPSyncStatus } from "../../types";
import { useRest, extractErrorMessage, formatLocalDateTime } from "../../utils";

const AutomaticOfflineTimeSet: FC = () => {
	const [localTime, setLocalTime] = useState<string>(new Date().toLocaleTimeString("en-US", { hour12: true }));
	const { loadData, data: ntpStatus } = useRest<NTPStatus>({ read: NTPApi.readNTPStatus });
	const { enqueueSnackbar } = useSnackbar();
	const lastSyncStatus = useRef(ntpStatus ? ntpStatus.status : null);

	const setDeviceTimeToCurrent = useCallback(async () => {
		try {
			// Utilize formatLocalDateTime to format the client's current local time appropriately
			const formattedTime = formatLocalDateTime(new Date());
			await NTPApi.updateTime({ local_time: formattedTime });
			enqueueSnackbar("Time has been set to the current local time automatically.", { variant: "success" });
		} catch (error: any) {
			enqueueSnackbar(extractErrorMessage(error, "Problem updating time"), { variant: "error" });
		}
	}, [enqueueSnackbar]);

	const checkTimeDifferenceAndUpdate = useCallback(async () => {
		if (!ntpStatus?.local_time) return;
		const deviceTime = new Date();
		const espTime = new Date(ntpStatus.local_time);
		const timeDifference = Math.abs(deviceTime.getTime() - espTime.getTime()) / 60000; // 60000 ms in a minute

		if (timeDifference > 1) {
			await setDeviceTimeToCurrent();
		}
	}, [ntpStatus?.local_time, setDeviceTimeToCurrent]);

	useEffect(() => {
		loadData().catch((error: any) => {
			enqueueSnackbar(extractErrorMessage(error, "Failed to fetch NTP status"), { variant: "error" });
		});
	}, [loadData, enqueueSnackbar]);

	useEffect(() => {
		console.log("useEffect Triggered");

		if (ntpStatus && ntpStatus.status !== lastSyncStatus.current) {
			if (ntpStatus.status === NTPSyncStatus.NTP_INACTIVE) {
				enqueueSnackbar("NTP is inactive. Automatic time setting is active.", { variant: "warning" });
				checkTimeDifferenceAndUpdate();
			}
			lastSyncStatus.current = ntpStatus.status;
		}
	}, [ntpStatus, checkTimeDifferenceAndUpdate, enqueueSnackbar]);

	useEffect(() => {
		const intervalId = setInterval(() => {
			setLocalTime(new Date().toLocaleTimeString("en-US", { hour12: true }));
		}, 1000);

		return () => clearInterval(intervalId);
	}, []);

	return (
		<Box pb={3}>
			<Typography variant="h6">Device Local Time: {localTime}</Typography>
			{ntpStatus && ntpStatus.status === NTPSyncStatus.NTP_INACTIVE && (
				<Typography variant="body2" color="text.secondary">
					NTP is currently inactive. Local time is being set automatically.
				</Typography>
			)}
		</Box>
	);
};

export default AutomaticOfflineTimeSet;
