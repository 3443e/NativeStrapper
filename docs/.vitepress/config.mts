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
            { text: 'Get Started', link: '/installation/' },
            { text: 'Building', link: '/building/' },
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
            '/installation/': [
                {
                    text: 'Installation',
                    items: [
                        { text: 'Overview', link: '/installation/' }
                    ]
                }
            ],

            '/building/': [
                {
                    text: 'Building',
                    items: [
                        { text: 'Overview', link: '/building/' },
                        { text: 'Linux', link: '/building/linux' },
                        { text: 'Windows', link: '/building/windows' }
                    ]
                }
            ],

            '/scripts/': [
                {
                    text: 'Writing Scripts',
                    items: [
                        { text: 'Overview', link: '/scripts/' },
                        { text: 'Metadata', link: '/scripts/metadata' },
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