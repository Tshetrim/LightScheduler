import { FC } from "react";
import { Navigate, Routes, Route } from "react-router-dom";

import AutoLightProject from "./AutoLightProject";

const ProjectRouting: FC = () => {
	return (
		<Routes>
			{
				// Add the default route for your project below
			}
			<Route path="/*" element={<Navigate to="autolight/information" />} />
			{
				// Add your project page routes below.
			}
			<Route path="autolight/*" element={<AutoLightProject />} />
		</Routes>
	);
};

export default ProjectRouting;
