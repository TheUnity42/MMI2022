<script>
    import Trash from "./Icons/Trash.svelte";
    import Pencil from "./Icons/Pencil.svelte";
    import { sensorStore } from "../stores/SensorStore.js";

    let removeAction = () => {
        console.log("removing sensor");
        sensorStore.update((sensors) => {
            sensors = sensors.filter((sensor) => sensor !== name);
            return sensors;
        });
    };

    let editAction = () => {
        editName = name;
        editing = !editing;
        // update the sensor store with the new name
        sensorStore.update((sensors) => {
            sensors = sensors.map((sensor) => {
                if (sensor === name) {
                    return editName;
                }
                return sensor;
            });
            return sensors;
        });
    };

    let editing = false;

    export let name;
    let editName;
</script>

<input type="checkbox" id="{name}-sensor-modality" class="modal-toggle" />
<div class="modal">
	<div class="modal-box">
        <label for="{name}-sensor-modality" class="btn btn-sm btn-circle absolute right-2 top-2">✕</label>
		<div class="flex flex-row gap-3">
            {#if editing}
                <input class="font-bold text-lg input input-bordered" type="text" bind:value={editName} />
            {:else}
                <h2 class="font-bold text-lg">{name}</h2>
            {/if}
            <button on:click={() => editing = !editing}><Pencil /></button>
        </div>
        <p class="h-32"></p>
		<button class="btn btn-square btn-error" on:click={removeAction} ><Trash/></button>
	</div>
</div>
