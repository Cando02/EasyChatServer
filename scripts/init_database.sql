use easychat;
# 用户表
create table if not exists users(
                                    id int primary key auto_increment comment '用户ID',
                                    username varchar(50) not null unique comment '用户名',
    password varchar(50) not null comment '密码（MD5加密）',
    nickname varchar(50) comment '昵称',
    avatar varchar(255) comment '头像URL',
    status tinyint default 0 comment '状态：0-离线，1-在线',
    created_at timestamp default current_timestamp comment '创建时间',
    updated_at timestamp default current_timestamp on update current_timestamp comment '更新时间',
    index idx_username(username),
    index idx_status(status)
    )engine = InnoDB default charset =utf8mb4 comment ='用户表';
# 消息表
create table if not exists messages(
                                       id int primary key auto_increment comment '消息ID',
                                       sender_id int not null comment '发送者ID',
                                       receiver_id int not null comment '接收者ID',
                                       content text not null comment '消息内容',
                                       message_type tinyint default 0 comment '消息类型：0-文本，1-图片，2-文件',
                                       is_offline tinyint default 0 comment '是否为离线消息：0-否，1-是',
                                       is_read tinyint default 0 comment '是否已读：0-未读，1-已读',
                                       created_at timestamp default current_timestamp comment '发送时间',
                                       index idx_sender(sender_id),
    index idx_receiver(receiver_id),
    index idx_offline(receiver_id,is_offline),
    foreign key (sender_id) references users(id) on delete cascade ,
    foreign key (receiver_id) references users(id) on delete cascade
    ) engine = InnoDB default charset =utf8mb4 comment ='消息表';
# 在线用户表（缓存表）
create table if not exists online_users(
                                           user_id int primary key comment '用户ID',
                                           socket_fd int not null comment 'Socket文件描述符',
                                           ip varchar(50) comment 'IP地址',
    port int comment '端口',
    last_heartbeat timestamp default current_timestamp on update current_timestamp comment '最后心跳时间',
    index idx_heartbeat(last_heartbeat),
    foreign key (user_id) references users(id) on delete cascade
    )engine =InnoDB default charset =utf8mb4 comment='在线用户表';
insert into users(username,password,nickname,status) values
                                                         ('admin','e10adc3949ba59abbe56e057f20f883e','管理员',0),
                                                         ('user1','e10adc3949ba59abbe56e057f20f883e','用户1',0),
                                                         ('user2','e10adc3949ba59abbe56e057f20f883e','用户2',0);