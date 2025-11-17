import { defineConfig } from 'vitepress'

// https://vitepress.dev/reference/site-config
export default defineConfig({
  title: "MCompass",
  description: "A Compass From Minecraft",
  head: [
    [
      'script',
      { 
        async: true, 
        src: 'https://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js?client=ca-pub-1189115759021088', 
        crossorigin: 'anonymous' 
      }
    ],
    [
      'script',
      {},
      `
        (function() {
          const path = window.location.pathname;
          if (path.startsWith('/') || path.startsWith('/en/')) {
            return; // 已在对应语言页面，不跳转
          }
          // 获取浏览器语言
          const userLang = navigator.language || navigator.userLanguage;
          // 中文优先跳转到 /，其他默认 /en/
          const defaultLang = userLang.includes('zh') ? '/' : '/en/';
          // 执行跳转
          window.location.href = defaultLang;
          console.log('Redirecting to: ' + defaultLang); // 修复拼写错误
        })();
      `
    ]
  ],
  locales: {
    root: {
      label: '简体中文',
      lang: 'zh-CN',
      themeConfig: {
        nav: [
          { text: 'Home', link: '/' },
        ],
        sidebar: [
          {
            text: '目录',
            items: [
              { text: '项目介绍', link: '/introduction' },
              { text: '物料清单', link: '/bom' },
              { text: '制作', link: '/make' },
              { text: 'API', link: '/api' },
              { text: '更新记录', link: '/update' },
              { text: '支持本项目', link: '/donate' }
            ]
          }
        ],
        socialLinks: [
          { icon: 'github', link: 'https://github.com/chaosgoo/mcompass' }
        ],
      },
    },
    en: {
      label: 'English',
      lang: 'en',
      link: '/en',
      themeConfig: {
        nav: [
          { text: 'Home', link: '/en/' },
        ],
        sidebar: [
          {
            text: 'Contents',
            items: [
              { text: 'Introduction', link: '/en/introduction' },
              // { text: 'Materials', link: '/en/bom' },
              { text: 'Making Guide', link: '/en/make' },
              { text: 'API', link: '/en/api' },
              { text: 'Changelog', link: '/en/update' },
              { text: 'Support This Project', link: '/en/donate' }
            ]
          }
        ],
        socialLinks: [
          { icon: 'github', link: 'https://github.com/chaosgoo/mcompass' }
        ],
    }
  }},
})