# GMSidebar

高性能、多线程的 BDS 侧边栏插件。

## 命令

- `/gmsidebar [true/false]` - 开关自己的侧边栏
- `/gmsidebar cache` - 清除自己的缓存
- `/gmsidebar disable` - 禁用侧边栏模组
- `/gmsidebar enable` - 启用侧边栏模组
- `/gmsidebar reload` - 重载侧边栏配置和数据文件
- `/gmsidebar as <player> [true/false]` - 开关指定玩家的侧边栏
- `/gmsidebar as <player> status` - 查看指定玩家的侧边栏状态
- `/gmsidebar as <player> cache` - 清除指定玩家的缓存

## 接口导出

> 注: 命名空间皆为`GMSidebar`

- `isEnable` - 侧边栏模组是否启用
- `enable` - 启用侧边栏模组
- `disable` - 禁用侧边栏模组
- `loadConfig` - 加载侧边栏配置文件
- `saveConfig` - 保存侧边栏配置文件
- `loadConfigFromPath` - 从指定路径加载侧边栏配置文件
  - path: string - 配置文件路径
- `saveConfigToPath` - 保存侧边栏配置文件到指定路径
  - path: string - 配置文件路径
- `loadData` - 加载侧边栏数据文件
- `saveData` - 保存侧边栏数据文件
- `loadDataFromPath` - 从指定路径加载侧边栏数据文件
  - path: string - 数据文件路径
- `saveDataToPath` - 保存侧边栏数据文件到指定路径
  - path: string - 数据文件路径
- `isPlayerSidebarEnabled` - 指定玩家的侧边栏是否启用
  - uuid: string - 玩家 UUID
- `setPlayerSidebarEnabled` - 设置指定玩家的侧边栏启用状态
  - uuid: string - 玩家 UUID
  - enabled: boolean - 是否启用侧边栏
- `clearPlayerCache` - 清除指定玩家的缓存
  - uuid: string - 玩家 UUID
- `clearAllPlayerCache` - 清除所有玩家的缓存

## Placeholder

> 注: 仅在侧边栏中生效

- `currentIndex` - 当前选中的索引
- `rawContent` - 原始内容
- `updateInterval` - 更新间隔
- `contentSize` - 索引长度

## 开源许可

### 源代码可用性

- 您可以自由地获取、使用和修改本插件的源代码，无论是个人使用还是商业目的。

### 修改发布

- 如果您对本插件进行了修改或衍生创作，并打算分发、发布该修改或衍生作品，您必须开源并且以 GPL3.0 协议下相同的许可证条件进行分发。

### 版权声明

- 在您分发或发布基于 GPL3.0 协议的软件时（包括但不限于本插件以及本插件的衍生作品），您必须保留原始版权声明、许可证说明和免责声明。

### 引用链接

- 如果您在一个作品中使用了本插件或者本插件的源码，您需要提供一个明确的引用链接，指向软件的许可证和源代码。

### 对整体的影响

- 如果您将基于本插件与其他插件结合使用，或整合成一个单一的插件，那么整个插件都需要遵守 GPL3.0 协议进行开源。
