import { writable } from "svelte/store";

export let sensorStore = writable(['Sensor 1', 'Sensor 2', 'Sensor 3', 'Sensor 4']);

