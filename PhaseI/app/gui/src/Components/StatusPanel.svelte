<script>
	import StatusElement from './StatusElement.svelte';
    import { sensorStore } from '../stores/SensorStore.js';
</script>

<div class="flex flex-col gap-1">
	{#each $sensorStore as sensor}
		<StatusElement name={sensor} />
	{/each}
</div>
