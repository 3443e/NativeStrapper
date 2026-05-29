import { defineConfig } from 'vitepress'

export default defineConfig({
    outDir: '.vitepress/dist',
    title: "NativeStrapper",
    description: "A cross-platform Roblox bootstrapper",
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
            { text: 'Writing Scripts', link: '/scripts/' },
            {
                text: 'Community',
                items: [
                    { text: 'GitHub', link: 'https://github.com/3443e/NativeStrapper' },
                    { text: 'Discord', link: 'https://discord.gg/eUSGaxZSBX' }
                ]
            }
        ],

        sidebar: {
            '/scripts/': [
                {
                    text: 'Writing Scripts',
                    items: [
                        { text: 'Overview', link: '/scripts/' },
                        { text: 'metadata table', link: '/scripts/metadata' },
                        { text: 'API Reference', link: '/scripts/api' },
                        { text: 'Examples', link: '/scripts/examples' }
                    ]
                }
            ]
        },

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