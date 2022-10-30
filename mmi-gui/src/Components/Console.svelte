<script>
	// import the console text
	import { consoleText } from '../lib/ApplicationContext.js';
	import { afterUpdate } from 'svelte';

	// start a child process
	// import { spawn } from 'child_process';

	// spawn the ../lib/testServer executable
	// const child = spawn('./src/lib/testServer');

	// // when the child process sends data, add it to the console text
	// child.stdout.on('data', (data) => {
	// 	$consoleText = $consoleText + data.toString();
	// });	

	// // make a function that adds a new line to the console every second
	// let interval = setInterval(() => {
	// 	$consoleText = $consoleText + 'Sample Text\n';
	// }, 500);

    // hold the pre element so we can force it to scroll to the bottom
    let element;

	afterUpdate(() => {
		scrollToBottom(element);
	});

	const scrollToBottom = async (node) => {
		node.scroll({ top: node.scrollHeight, behavior: 'smooth' });
	};
</script>

<!-- display the console text in a scrollable code block -->
<pre
	bind:this={element}
	class="bg-gray-800 text-white p-4 overflow-auto justify-end w-full h-full">{$consoleText}
</pre>
