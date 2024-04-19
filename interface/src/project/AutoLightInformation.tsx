import { FC } from "react";
import { Typography, Box, List, ListItem, ListItemText } from "@mui/material";
import { SectionContent } from "../components";

const AutoLightInformation: FC = () => (
	<SectionContent title="AutoLight Instructions" titleGutter>
		<Typography variant="body1" paragraph>
			Welcome to the <strong>AutoLight Scheduler</strong>. Follow these guidelines below for the best experience
			with the system.
		</Typography>

		<Typography variant="h6" gutterBottom>
			Getting Started / Miscellaneous
		</Typography>
		<List>
			<ListItem>
				<ListItemText
					primary={
						<span>
							Connect the device to <strong style={{ color: "#1976d2" }}>WiFi</strong> for the best
							experience by going to <strong style={{ color: "#1976d2" }}>Wifi Connection</strong> in the
							sidebar, scanning networks, and connecting to a network. This ensures easy and accurate
							access to the AutoLight portal via the IP address provided in the
							<strong style={{ color: "#1976d2" }}> WiFi connection settings tab</strong>.
						</span>
					}
				/>
			</ListItem>
			<ListItem>
				<ListItemText
					primary={
						<span>
							If offline, connect directly to the device's Access Point by selecting this device's network
							name (SSID) on your device's WiFi settings. You might need to open{" "}
							<strong style={{ color: "#1976d2" }}>neverssl.com</strong> in your browser or wait for the
							captive portal to navigate to the ESP's website automatically.
						</span>
					}
				/>
			</ListItem>
			<ListItem>
				<ListItemText primary="Without a WiFi connection, regularly update the local time in the network settings tab to maintain schedule accuracy." />
			</ListItem>
		</List>

		<Typography variant="h6" gutterBottom>
			Operating the Light Schedule
		</Typography>
		<List>
			<ListItem>
				<ListItemText
					primary={
						<span>
							Set or edit light schedules via the{" "}
							<strong style={{ color: "#1976d2" }}>Light Settings</strong> tab. You can delete old
							schedules or modify them as needed.
						</span>
					}
				/>
			</ListItem>
			<ListItem>
				<ListItemText primary="Any manual color change will be overridden by the active schedule. If no schedule is set to follow, the last color remains active." />
			</ListItem>
			<ListItem>
				<ListItemText
					primary={
						<span>
							Admin privileges are required to change{" "}
							<strong style={{ color: "#1976d2" }}>pin settings</strong>. These settings determine the
							hardware connections of the light system.
						</span>
					}
				/>
			</ListItem>
		</List>

		<Typography variant="body2" paragraph>
			Happpy Scheduling!
		</Typography>
	</SectionContent>
);

export default AutoLightInformation;
