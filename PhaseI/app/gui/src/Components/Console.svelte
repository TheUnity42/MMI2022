<script>
    let messages = ['Test Message 1', 'Test Message 2', 'Test Message 3'];
</script>

<div class="bg-neutral w-full h-full rounded-md shadow-md">
    {#each messages as message}
        <div class="px-2">
            <p class="text-white">$> {message}</p>
        </div>
    {/each}
</div>