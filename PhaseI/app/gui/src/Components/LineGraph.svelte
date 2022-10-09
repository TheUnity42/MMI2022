<script>
	import { graphWidth, graphHeight } from '../stores/GraphStore.js';
	export let xrng;
	export let yrng;

	// generate random y data and sequential x data
	const data = [];
	for (let i = 0; i < xrng; i++) {
		data.push({
			x: i,
			y: Math.random() * yrng
		});
	}

	console.log($graphWidth, $graphHeight);
	let svgData = [];
	// map data to svg coordinates
	if ($graphWidth && $graphHeight) {
		svgData = data.map((d) => {
			return {
				x: (d.x * $graphWidth) / xrng,
				y: (d.y * $graphHeight) / yrng
			};
		});
	}
	// construct an svg path string from the data
	const path = svgData.reduce((acc, d) => {
		return acc + `L${d.x} ${d.y} `;
	}, 'M0 0 ');
</script>

<!-- Draw an SVG Path -->
<path d={path} stroke="black" fill="none" />
