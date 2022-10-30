// create a svelte store for the console text

import { writable } from 'svelte/store';
import dgram from 'dgram';

const ConsoleSocket = () => {
    
}


// store console text
export const consoleText = writable('');
