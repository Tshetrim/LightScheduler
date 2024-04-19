import React, { FC } from "react";
import { Navigate, Route, Routes } from "react-router-dom";

import { Tab } from "@mui/material";
import { RouterTabs, useRouterTab, useLayoutTitle } from "../components";

import AutoLightInformation from "./AutoLightInformation";
import RGBLightStateRestForm from "./RGBLightStateRestForm";

const AutoLightProject: FC = () => {
	useLayoutTitle("AutoLight Project");
	const { routerTab } = useRouterTab();

	return (
		<>
			<RouterTabs value={routerTab}>
				<Tab value="information" label="Information" />
				<Tab value="rest" label="Light Settings" />
				{/* <Tab value="socket" label="WebSocket Example" /> */}
			</RouterTabs>
			<Routes>
				<Route path="information" element={<AutoLightInformation />} />
				<Route path="rest" element={<RGBLightStateRestForm />} />
				<Route path="/*" element={<Navigate replace to="information" />} />
			</Routes>
		</>
	);
};

export default AutoLightProject;
