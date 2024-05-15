import parseMilliseconds from "parse-ms";

const LOCALE_FORMAT = new Intl.DateTimeFormat([...window.navigator.languages], {
	day: "numeric",
	month: "short",
	year: "numeric",
	hour: "numeric",
	minute: "numeric",
	second: "numeric",
	hour12: false,
});

export const formatDateTime = (dateTime: string) => {
	return LOCALE_FORMAT.format(new Date(dateTime.substr(0, 19)));
};

export const formatLocalDateTime = (date: Date) => {
	return new Date(date.getTime() - date.getTimezoneOffset() * 60000).toISOString().slice(0, -1).substr(0, 19);
};

export const formatDuration = (duration: number) => {
	const { days, hours, minutes, seconds } = parseMilliseconds(duration * 1000);
	var formatted = "";
	if (days) {
		formatted += pluralize(days, "day");
	}
	if (formatted || hours) {
		formatted += pluralize(hours, "hour");
	}
	if (formatted || minutes) {
		formatted += pluralize(minutes, "minute");
	}
	if (formatted || seconds) {
		formatted += pluralize(seconds, "second");
	}
	return formatted;
};

export const epochToLocalDateTime = (epochSeconds: number) => {
	const date = new Date(epochSeconds * 1000);
	return formatLocalDateTime(date);
};

export const localDateTimeToEpoch = (localDateTime: string | Date) => {
	const date = new Date(localDateTime);
	return Math.floor(date.getTime() / 1000);
};

export const isEpochTimePast = (endTimeEpoch: number) => {
	const currentTime = new Date().getTime() / 1000;
	return endTimeEpoch < currentTime;
};

export const isEpochTimeActive = (startTimeEpoch: number, endTimeEpoch: number) => {
	const currentTime = Math.floor(Date.now() / 1000);
	return startTimeEpoch <= currentTime && endTimeEpoch >= currentTime;
};

export const isTimeWithin24HourWindow = (startTimeEpoch: number, endTimeEpoch: number) => {
	const currentTime = new Date();
	const startDate = new Date(startTimeEpoch * 1000);
	const endDate = new Date(endTimeEpoch * 1000);

	const currentSeconds = currentTime.getHours() * 3600 + currentTime.getMinutes() * 60 + currentTime.getSeconds();
	const startSeconds = startDate.getHours() * 3600 + startDate.getMinutes() * 60 + startDate.getSeconds();
	const endSeconds = endDate.getHours() * 3600 + endDate.getMinutes() * 60 + endDate.getSeconds();

	return currentSeconds >= startSeconds && currentSeconds <= endSeconds;
};

const pluralize = (count: number, noun: string, suffix: string = "s") =>
	` ${count} ${noun}${count !== 1 ? suffix : ""} `;
