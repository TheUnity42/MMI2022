/** @type {import('tailwindcss').Config} */
module.exports = {
	content: ['./src/**/*.{html,js,svelte,ts}'],
	theme: {
		extend: {}
	},
	plugins: [require('daisyui')],
	// daisyui: {
	// 	themes: [
	// 		{
	// 			mmitheme: {
	// 				primary: '#d946ef',
	// 				secondary: '#a855f7',
	// 				accent: '#37CDBE',
	// 				neutral: '#1f2937',
	// 				'base-100': '#374151',
	// 				info: '#3ABFF8',
	// 				success: '#36D399',
	// 				warning: '#FBBD23',
	// 				error: '#F87272'
	// 			}
	// 		}
	// 	]
	// }
};
