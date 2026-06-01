import { defineConfig } from 'vitepress'

export default defineConfig({
    outDir: '.vitepress/dist',
    title: "NativeStrapper",
    description: "A cross-platform and very modular ROBLOX bootstrapper.",
    head: [
        ['link', { rel: 'icon', type: 'image/png', href: '/icon.png' }]
    ],

    themeConfig: {
        
        logo: {
            light: '/logo-light.png',
            dark: '/logo-dark.png'
        },

        siteTitle: false,

        nav: [
            { text: 'Home', link: '/' },
            { text: 'Installation', link: '/installation/' },
            { text: 'Writing Scripts', link: '/scripts/' },
            {
                text: 'Community',
                items: [
                    { text: 'GitHub', link: 'https://github.com/3443e/NativeStrapper' },
                    { text: 'Discord', link: 'https://discord.gg/eUSGaxZSBX' }
                ]
            }
        ],

        sidebar: [
            {
                text: 'Getting Started',
                collapsed: false,
                items: [
                    { text: 'Installation', link: '/installation/' },
                    { text: 'Building from Source', link: '/building/' }
                ]
            },

            {
                text: 'Bootstrap Scripts',
                collapsed: false,
                items: [
                    { text: 'Overview', link: '/scripts/' },
                    { text: 'Metadata Table', link: '/scripts/metadata' },
                    { text: 'API Reference', link: '/scripts/api' },
                    { text: 'Examples', link: '/scripts/examples' }
                ]
            },

            {
                text: 'Building',
                collapsed: false,
                items: [
                    { text: 'Index', link: '/building/' },
                    { text: 'Linux (x86_64)', link: '/building/linux' },
                    { text: 'Windows (x64)', link: '/building/windows' }
                ]
            },

            {
                text: 'Community',
                collapsed: true,
                items: [
                    { text: 'GitHub Repository', link: 'https://github.com/3443e/NativeStrapper' },
                    { text: 'Discord Server', link: 'https://discord.gg/eUSGaxZSBX' }
                ]
            }
        ],

        socialLinks: [
            { icon: 'github', link: 'https://github.com/3443e/NativeStrapper' },
            { icon: 'discord', link: 'https://discord.gg/eUSGaxZSBX' }
        ],

        footer: {
            message: 'Made with love -3443'
        }
    },

    appearance: 'dark'
})