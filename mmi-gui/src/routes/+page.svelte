<script>
	import Console from '../Components/Console.svelte';
	export let title;
</script>

<div class="grid grid-cols-2 gap-2 h-full w-full">
	<h1>{title}</h1>
	<div class="h-96 row-span-full">
		<h1 class="font-sans italic text-xl text-right">Console</h1>
		<Console />
	</div>
	<div>
		<h1 class="font-sans italic text-xl text-right">Sensor plots</h1>
	</div>
	<div>
		<h1 class="font-sans italic text-xl text-right">Controls</h1>
	</div>
</div>
