import { defineConfig } from 'vitepress'

// https://vitepress.dev/reference/site-config
export default defineConfig({
  title: "MCompass",
  description: "A Compass From Minecraft",
  themeConfig: {
    nav: [
      { text: 'Home', link: '/' },
    ],

    sidebar: [
      {
        text: '目录',
        items: [
          { text: 'Readme', link: '/readme' },
          { text: 'API', link: '/api' },
          { text: '制作', link: '/make' },
          { text: '更新记录', link: '/update' },
          { text: '支持本项目', link: '/donate' }
        ]
      }
    ],

    socialLinks: [
      { icon: 'github', link: 'https://github.com/chaosgoo/mcompass' }
    ],
  },
  head: [
    ['script', { async: true, src: 'https://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js?client=ca-pub-1189115759021088', crossorigin: 'anonymous' }]
  ]
})