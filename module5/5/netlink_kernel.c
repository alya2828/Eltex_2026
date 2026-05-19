#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>
#include <linux/string.h>

#define NETLINK_USER 31
#define MAX_MSG_SIZE 10

struct sock *nl_sk = NULL;// Сокет netlink в ядре

//Функция обработки входящего сообщения от userspace
static void recv_message(struct sk_buff *skb)
{

    struct nlmsghdr *nlh; // заголовок netlink сообщения
    int pid;
    struct sk_buff *skb_out;// буфер ответа
    int msg_size;// размер сообщения
    char *msg = "OK";// ответ пользователю
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    // получаем сообщение от userspace
    nlh = nlmsg_hdr(skb);

    printk(KERN_INFO "Netlink received msg payload: %s\n",
           (char *)nlmsg_data(nlh));

    // pid процесса отправителя
    pid = nlh->nlmsg_pid;

    // размер ответа
    msg_size = strlen(msg);

    // создаём буфер ответа kernel -> user
    skb_out = nlmsg_new(msg_size, GFP_KERNEL);

    if (!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    // Формируем netlink сообщение
    nlh = nlmsg_put(
        skb_out,
        0,
        0,
        NLMSG_DONE,
        msg_size,
        0
    );

    // Указываем что это unicast (не multicast)
    NETLINK_CB(skb_out).dst_group = 0;

    // копируем данные в payload
    memcpy(nlmsg_data(nlh), msg, strlen(msg));

    // отправляем ответ userspace процессу
    res = nlmsg_unicast(nl_sk, skb_out, pid);
    printk(KERN_INFO "send result: %d\n", res);

    if (res < 0)
        printk(KERN_INFO "Error while sending back to user\n");
}

// Конфигурация netlink socket
struct netlink_kernel_cfg cfg = {
    .input = recv_message,
};
// Инициализация модуля
static int __init recv_message_init(void)
{
    printk("Entering: %s\n", __FUNCTION__);

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

    if (!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

// Выгрузка модуля
static void __exit recv_message_exit(void)
{
    printk(KERN_INFO "exiting hello module\n");

    netlink_kernel_release(nl_sk);
}

module_init(recv_message_init);
module_exit(recv_message_exit);

MODULE_LICENSE("GPL");
