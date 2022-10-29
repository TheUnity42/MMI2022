<script>
	import StatusPanel from '../Components/StatusPanel.svelte';
	import HelpModality from '../Components/HelpModality.svelte';
	import SensorDrawer from '../Components/SensorDrawer.svelte';
    import Graph from '../Components/Graphing/Graph.svelte';
	import LineGraph from '../Components/Graphing/LineGraph.svelte';
	import Console from '../Components/Console.svelte';
</script>

<SensorDrawer>
	<div class="p-3 pl-4 overflow-x-clip">
		<h1 class="text-3xl font-semibold">Man Muscle Sensor Dashboard</h1>
		<label 
			for="help-modality"
			class="absolute top-3 right-3 btn btn-info btn-square btn-sm rounded-md">
			?
		</label>
		<HelpModality />
		<div class="absolute left-0 h-1/2 w-2/3 p-5">
			<!-- <div class="bg-slate-800 w-full h-full" /> -->
            <Graph>
				<LineGraph xrng="100" yrng="4" />
			</Graph>
		</div>
		<div class="absolute right-0 h-1/2 w-1/3 p-5">
			<StatusPanel />
		</div>
		<div class="absolute bottom-5 left-5">
			<label for="sensor-drawer" class="btn btn-primary px-8">Manage Sensors</label>
		</div>
		<div class="absolute bottom-5 right-5 h-1/3 w-1/2 px-5">
			<Console />
		</div>
	</div>
</SensorDrawer>
